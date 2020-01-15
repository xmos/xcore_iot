// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include "soc.h"
#include "soc_bsp_common.h"
#include "bitstream_devices.h"

#include "ethernet_driver.h"

#if ( SOC_ETHERNET_PERIPHERAL_USED == 0 )
#define BITSTREAM_ETHERNET_DEVICE_COUNT 0
soc_peripheral_t bitstream_ethernet_devices[BITSTREAM_ETHERNET_DEVICE_COUNT];
#endif /* SOC_ETHERNET_PERIPHERAL_USED */

void ethernet_driver_send_packet(
        soc_peripheral_t dev,
        void *packet,
        unsigned n)
{
    soc_dma_ring_buf_t *tx_ring_buf = soc_peripheral_tx_dma_ring_buf(dev);
    soc_dma_ring_tx_buf_set(tx_ring_buf, packet, n);
    soc_peripheral_hub_dma_request(dev, SOC_DMA_TX_REQUEST);
}

void ethernet_driver_send_packet_wait_for_copy(
        soc_peripheral_t dev,
        void *packet,
        unsigned n)
{
    soc_dma_ring_buf_t *tx_ring_buf = soc_peripheral_tx_dma_ring_buf(dev);
    soc_dma_ring_tx_buf_set(tx_ring_buf, packet, n);
    soc_peripheral_hub_dma_request(dev, SOC_DMA_TX_REQUEST);

    while (( soc_dma_ring_tx_buf_get(tx_ring_buf, NULL, NULL)) != packet ) {
        ;   /* Wait until the packet we just sent is recieved */
    }
}

void ethernet_get_mac_addr(
        soc_peripheral_t dev,
        size_t ifnum,
        uint8_t mac_address[ETHERNET_MACADDR_NUM_BYTES])
{
    chanend c = soc_peripheral_ctrl_chanend(dev);

    soc_peripheral_function_code_tx(c, ETH_DEV_GET_MAC_ADDR);

    soc_peripheral_varlist_tx(
            c, 1,
            sizeof(ifnum), &ifnum);

    soc_peripheral_varlist_rx(
            c, 1,
            ETHERNET_MACADDR_NUM_BYTES, mac_address);
}

void ethernet_set_mac_addr(
        soc_peripheral_t dev,
        size_t ifnum,
        const uint8_t mac_address[ETHERNET_MACADDR_NUM_BYTES])
{
    chanend c = soc_peripheral_ctrl_chanend(dev);

    soc_peripheral_function_code_tx(c, ETH_DEV_SET_MAC_ADDR);

    soc_peripheral_varlist_tx(
            c, 2,
            sizeof(ifnum), &ifnum,
            ETHERNET_MACADDR_NUM_BYTES, mac_address);
}

void ethernet_driver_smi_write_reg(
        soc_peripheral_t dev,
        uint8_t phy_addr,
        uint8_t reg_addr,
        uint16_t val)
{
    chanend c = soc_peripheral_ctrl_chanend(dev);

    soc_peripheral_function_code_tx(c, ETH_DEV_SMI_WRITE_REG);

    soc_peripheral_varlist_tx(
            c, 3,
            sizeof(phy_addr), &phy_addr,
            sizeof(reg_addr), &reg_addr,
            sizeof(val), &val);
}

uint16_t ethernet_driver_smi_read_reg(
        soc_peripheral_t dev,
        uint8_t phy_addr,
        uint8_t reg_addr)
{
    chanend c = soc_peripheral_ctrl_chanend(dev);
    uint16_t retVal;

    soc_peripheral_function_code_tx(c, ETH_DEV_SMI_READ_REG);

    soc_peripheral_varlist_tx(
            c, 2,
            sizeof(phy_addr), &phy_addr,
            sizeof(reg_addr), &reg_addr);

    soc_peripheral_varlist_rx(
            c, 1,
            sizeof(retVal), &retVal);

    return retVal;
}

ethernet_link_state_t ethernet_driver_smi_get_link_state(
        soc_peripheral_t dev,
        uint8_t phy_addr)
{
    chanend c = soc_peripheral_ctrl_chanend(dev);
    ethernet_link_state_t retVal;

    soc_peripheral_function_code_tx(c, ETH_DEV_SMI_GET_LINK_STATUS);

    soc_peripheral_varlist_rx(
            c, 1,
            sizeof(retVal), &retVal);

    return retVal;
}

soc_peripheral_t ethernet_driver_init(
        int device_id,
        int rx_desc_count,
        int rx_buf_size,
        int tx_desc_count,
        void *app_data,
        int isr_core,
        rtos_irq_isr_t isr)
{
    soc_peripheral_t device;

    xassert(device_id >= 0 && device_id < BITSTREAM_ETHERNET_DEVICE_COUNT);

    device = bitstream_ethernet_devices[device_id];

    soc_peripheral_common_dma_init(
            device,
            rx_desc_count,
            rx_buf_size,
            tx_desc_count,
            app_data,
            isr_core,
            isr);

    return device;
}
