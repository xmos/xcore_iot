// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <platform.h>
#include <timer.h>
#include <string.h>

#include "soc.h"
#include "xassert.h"
#include "eth_dev.h"


void eth_dev_init(
        client ethernet_cfg_if ?i_eth_cfg,
        client ethernet_rx_if ?i_eth_rx,
        otp_ports_t &?otp_ports,
        const char (&?mac_address0)[6])
{
    char mac_address[6];

    if (!isnull(mac_address0)) {
      memcpy(mac_address, mac_address0, 6);
    } else if (!isnull(otp_ports)) {
      otp_board_info_get_mac(otp_ports, 0, mac_address);
    } else if (!isnull(i_eth_cfg)) {
      i_eth_cfg.get_macaddr(0, mac_address);
    } else {
      fail("Must supply OTP ports or MAC address to xtcp component");
    }

    if (!isnull(i_eth_cfg)) {
      i_eth_cfg.set_macaddr(0, mac_address);

      size_t index = i_eth_rx.get_index();
      ethernet_macaddr_filter_t macaddr_filter;
      memcpy(macaddr_filter.addr, mac_address, sizeof(mac_address));
      i_eth_cfg.add_macaddr_filter(index, 0, macaddr_filter);

      // Add broadcast filter
      for (size_t i = 0; i < 6; i++)
        macaddr_filter.addr[i] = 0xff;
      i_eth_cfg.add_macaddr_filter(index, 0, macaddr_filter);

      // Only allow ARP and IP packets to the stack
      i_eth_cfg.add_ethertype_filter(index, 0x0806);
      i_eth_cfg.add_ethertype_filter(index, 0x0800);
    }
}

/*
 *  \param i_eth_cfg    If this component is connected to an MAC component
 *                      in the Ethernet library then this interface should be
 *                      used to connect to it. Otherwise it should be set to
 *                      null.
 *  \param i_eth_rx     If this component is connected to an MAC component
 *                      in the Ethernet library then this interface should be
 *                      used to connect to it. Otherwise it should be set to
 *                      null.
 *  \param i_eth_tx     If this component is connected to an MAC component
 *                      in the Ethernet library then this interface should be
 *                      used to connect to it. Otherwise it should be set to
 *                      null.
 *  \param i_smi        If this connection to an Ethernet SMI component is
 *                      then the XTCP component will poll the Ethernet PHY
 *                      for link up/link down events. Otherwise, it will
 *                      expect link up/link down events from the connected
 *                      Ethernet MAC.
 *  \param phy_address  The SMI address of the Ethernet PHY
 *  \param mac_address  If this array is non-null then it will be used to set
 *                      the MAC address of the component.
 *  \param otp_ports    If this port structure is non-null then the component
 *                      will obtain the MAC address from OTP ROM. See the OTP
 *                      reading library user guide for details.
 */
static unsafe void eth_dev_handler(
        chanend data_to_dma_c,
        chanend data_from_dma_c,
        chanend ?ctrl_c,
        client ethernet_cfg_if ?i_eth_cfg,
        client ethernet_rx_if ?i_eth_rx,
        client ethernet_tx_if ?i_eth_tx,
        client smi_if i_smi,
        uint8_t phy_address,
        const char (&?mac_address0)[6],
        otp_ports_t &?otp_ports)
{
    timer tmr;
    uint32_t time;
    int no_rx = 0, no_tx = 0;
    size_t frame_len;
    unsigned tx_ifnum;
    uint32_t cmd;
    ethernet_packet_info_t desc;
    uint8_t mac_address[MACADDR_NUM_BYTES];
    uint8_t frame_buf[ETHERNET_MAX_PACKET_SIZE];

    eth_dev_init(i_eth_cfg, i_eth_rx, otp_ports, mac_address0);

    while (smi_phy_is_powered_down(i_smi, phy_address));
    smi_configure(i_smi, phy_address, LINK_100_MBPS_FULL_DUPLEX, SMI_ENABLE_AUTONEG);

    while (1)
    {
        [[ordered]]
        select
        {
        case !isnull(ctrl_c) => soc_peripheral_function_code_rx(ctrl_c, &cmd):
            switch( cmd )
            {
            case ETH_DEV_GET_MAC_ADDR:
                soc_peripheral_varlist_rx(
                        ctrl_c, 1,
                        sizeof(tx_ifnum), &tx_ifnum);

                i_eth_cfg.get_macaddr(tx_ifnum, mac_address);

                soc_peripheral_varlist_tx(
                        ctrl_c, 1,
                        sizeof(mac_address), &mac_address);

                break;

            case ETH_DEV_SET_MAC_ADDR:
                uint8_t original_mac_address[MACADDR_NUM_BYTES];

                soc_peripheral_varlist_rx(
                        ctrl_c, 2,
                        sizeof(tx_ifnum), &tx_ifnum,
                        sizeof(mac_address), &mac_address);

                i_eth_cfg.get_macaddr(tx_ifnum, original_mac_address);

                i_eth_cfg.set_macaddr(tx_ifnum, mac_address);


                size_t index = i_eth_rx.get_index();
                ethernet_macaddr_filter_t macaddr_filter;

                /* Remove old MAC addr from filter */
                memcpy(macaddr_filter.addr, original_mac_address, sizeof(original_mac_address));
                i_eth_cfg.del_macaddr_filter(index, 0, macaddr_filter);

                /* Add new MAC addr to filter */
                memcpy(macaddr_filter.addr, mac_address, sizeof(mac_address));
                i_eth_cfg.add_macaddr_filter(index, 0, macaddr_filter);

                break;

            case ETH_DEV_SMI_READ_REG:
                uint16_t retVal;
                uint8_t phy_addr;
                uint8_t reg_addr;

                soc_peripheral_varlist_rx(
                        ctrl_c, 2,
                        sizeof(phy_addr), &phy_addr,
                        sizeof(reg_addr), &reg_addr);

                retVal = i_smi.read_reg(phy_addr, reg_addr);
                soc_peripheral_varlist_tx(
                        ctrl_c, 1,
                        sizeof(retVal), &retVal);
                break;

            case ETH_DEV_SMI_WRITE_REG:
                uint8_t phy_addr;
                uint8_t reg_addr;
                uint16_t val;

                soc_peripheral_varlist_rx(
                        ctrl_c, 3,
                        sizeof(phy_addr), &phy_addr,
                        sizeof(reg_addr), &reg_addr,
                        sizeof(val), &val);
                i_smi.write_reg(phy_addr, reg_addr, val);

                break;

            case ETH_DEV_SMI_GET_LINK_STATUS:
                ethernet_link_state_t link_status;

                link_status = smi_get_link_state(i_smi, phy_address);

                soc_peripheral_varlist_tx(
                        ctrl_c, 1,
                        sizeof(link_status), &link_status);
                break;

            default:
                /* Command is not valid */
                fail("Invalid cmd");
                break;
            }
            break;

        case !no_rx => xcore_freertos_dma_device_rx_ready(data_from_dma_c):

            frame_len = xcore_freertos_dma_device_rx_data(data_from_dma_c, frame_buf, sizeof(frame_buf));

            /* If the stack sends less than 60 bytes, pad with 0x00 for the MAC */
            if( frame_len < 60 )
            {
                memset( &frame_buf[frame_len], 0x00, (60 - frame_len) );
                frame_len = 60;
            }

            i_eth_tx.send_packet(frame_buf, frame_len, 0);

            no_rx = 1;
            tmr :> time;
            break;

        case !no_tx => i_eth_rx.packet_ready():

            i_eth_rx.get_packet(desc, frame_buf, ETHERNET_MAX_PACKET_SIZE);
            xcore_freertos_dma_device_tx_data(data_to_dma_c, frame_buf, desc.len);

            no_tx = 1;
            tmr :> time;
            break;

        case no_rx || no_tx => tmr when timerafter(time-1) :> void:
            /*
             * This acts as a guarded default if an RX or TX
             * happened on the previous iteration. It ensures that
             * if there are currently frames to both send and receive
             * that it round robins between the two rather than only
             * servicing one of them.
             */
            no_rx = 0;
            no_tx = 0;
            break;
        }
    }
}


void eth_dev(
        chanend data_to_dma_c,
        chanend data_from_dma_c,
        chanend ?ctrl_c,
        port p_eth_rxclk,
        port p_eth_rxerr,
        port p_eth_rxd,
        port p_eth_rxdv,
        port p_eth_txclk,
        port p_eth_txen,
        port p_eth_txd,
        port p_eth_timing,
        clock eth_rxclk,
        clock eth_txclk,
        port ?p_smi_mdio,
        port ?p_smi_mdc,
        otp_ports_t &?otp_ports)
{
    smi_if i_smi;

    ethernet_cfg_if i_cfg[1];
    ethernet_rx_if i_rx[1];
    ethernet_tx_if i_tx[1];

    par {
        if( ETHCONF_USE_RT_MAC )
        {
            mii_ethernet_rt_mac(i_cfg, 1, i_rx, 1, i_tx, 1,
                                NULL, NULL,
                                p_eth_rxclk, p_eth_rxerr, p_eth_rxd, p_eth_rxdv,
                                p_eth_txclk, p_eth_txen, p_eth_txd,
                                eth_rxclk, eth_txclk,
                                ETHCONF_RT_MII_RX_BUFSIZE,
                                ETHCONF_RT_MII_TX_BUFSIZE,
                                ETHERNET_DISABLE_SHAPER);
        }
        else
        {
            mii_ethernet_mac(i_cfg, 1, i_rx, 1, i_tx, 1,
                             p_eth_rxclk, p_eth_rxerr, p_eth_rxd, p_eth_rxdv,
                             p_eth_txclk, p_eth_txen, p_eth_txd, p_eth_timing,
                             eth_rxclk, eth_txclk, ETHCONF_MII_BUFSIZE);
        }

        // SMI/ethernet phy driver
        smi(i_smi, p_smi_mdio, p_smi_mdc);

        unsafe {
            eth_dev_handler(data_to_dma_c, data_from_dma_c, ctrl_c,
                            i_cfg[0],i_rx[0], i_tx[0],
                            i_smi, ETHCONF_SMI_PHY_ADDRESS,
                            null, otp_ports);
        }
    }
}

void eth_dev_smi_singleport(
        chanend data_to_dma_c,
        chanend data_from_dma_c,
        chanend ?ctrl_c,
        port p_eth_rxclk,
        port p_eth_rxerr,
        port p_eth_rxd,
        port p_eth_rxdv,
        port p_eth_txclk,
        port p_eth_txen,
        port p_eth_txd,
        port p_eth_timing,
        clock eth_rxclk,
        clock eth_txclk,
        port ?p_smi,
        otp_ports_t &?otp_ports)
{
    smi_if i_smi;

    ethernet_cfg_if i_cfg[1];
    ethernet_rx_if i_rx[1];
    ethernet_tx_if i_tx[1];

    par {
        if( ETHCONF_USE_RT_MAC )
        {
            mii_ethernet_rt_mac(i_cfg, 1, i_rx, 1, i_tx, 1,
                                NULL, NULL,
                                p_eth_rxclk, p_eth_rxerr, p_eth_rxd, p_eth_rxdv,
                                p_eth_txclk, p_eth_txen, p_eth_txd,
                                eth_rxclk, eth_txclk,
                                ETHCONF_RT_MII_RX_BUFSIZE,
                                ETHCONF_RT_MII_TX_BUFSIZE,
                                (ETHCONF_USE_SHAPER ? ETHERNET_ENABLE_SHAPER : ETHERNET_DISABLE_SHAPER));
        }
        else
        {
            mii_ethernet_mac(i_cfg, 1, i_rx, 1, i_tx, 1,
                             p_eth_rxclk, p_eth_rxerr, p_eth_rxd, p_eth_rxdv,
                             p_eth_txclk, p_eth_txen, p_eth_txd, p_eth_timing,
                             eth_rxclk, eth_txclk, ETHCONF_MII_BUFSIZE);
        }

        // SMI/ethernet phy driver
        smi_singleport(i_smi, p_smi, ETHCONF_SMI_MDIO_BIT_POS, ETHCONF_SMI_MDC_BIT_POS);

        unsafe {
            eth_dev_handler(data_to_dma_c, data_from_dma_c, ctrl_c,
                            i_cfg[0],i_rx[0], i_tx[0],
                            i_smi, ETHCONF_SMI_PHY_ADDRESS,
                            null, otp_ports);
        }
    }
}
