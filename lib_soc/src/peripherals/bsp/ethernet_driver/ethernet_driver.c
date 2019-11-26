// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <stdlib.h>

#include "soc.h"
#include "bitstream_devices.h"

#include "ethernet_driver.h"

void ethernet_driver_send_packet(
        xcore_freertos_device_t dev,
        void *packet,
        unsigned n)
{
    xcore_freertos_dma_ring_buf_t *tx_ring_buf = xcore_freertos_dma_device_tx_ring_buf(dev);
    xcore_freertos_dma_ring_buf_tx(tx_ring_buf, packet, n);
    xcore_freertos_dma_request();
}

void ethernet_driver_send_packet_wait_for_copy(
        xcore_freertos_device_t dev,
        void *packet,
        unsigned n)
{
    xcore_freertos_dma_ring_buf_t *tx_ring_buf = xcore_freertos_dma_device_tx_ring_buf(dev);
    xcore_freertos_dma_ring_buf_tx(tx_ring_buf, packet, n);
    xcore_freertos_dma_request();

    while (( xcore_freertos_dma_ring_buf_tx_complete(tx_ring_buf, NULL, NULL)) != packet ) {
        ;   /* Wait until the packet we just sent is recieved */
    }
}

void ethernet_get_mac_addr(
        xcore_freertos_device_t dev,
        size_t ifnum,
        uint8_t mac_address[ETHERNET_MACADDR_NUM_BYTES])
{
    chanend c = xcore_freertos_dma_device_ctrl_chanend(dev);

    xcore_freertos_periph_function_code_tx(c, ETH_DEV_GET_MAC_ADDR);

    xcore_freertos_periph_varlist_tx(
            c, 1,
            sizeof(ifnum), &ifnum);

    xcore_freertos_periph_varlist_rx(
            c, 1,
            ETHERNET_MACADDR_NUM_BYTES, mac_address);
}

void ethernet_set_mac_addr(
        xcore_freertos_device_t dev,
        size_t ifnum,
        const uint8_t mac_address[ETHERNET_MACADDR_NUM_BYTES])
{
    chanend c = xcore_freertos_dma_device_ctrl_chanend(dev);

    xcore_freertos_periph_function_code_tx(c, ETH_DEV_SET_MAC_ADDR);

    xcore_freertos_periph_varlist_tx(
            c, 2,
            sizeof(ifnum), &ifnum,
            ETHERNET_MACADDR_NUM_BYTES, mac_address);
}

void ethernet_driver_smi_write_reg(
        xcore_freertos_device_t dev,
        uint8_t phy_addr,
        uint8_t reg_addr,
        uint16_t val)
{
    chanend c = xcore_freertos_dma_device_ctrl_chanend(dev);

    xcore_freertos_periph_function_code_tx(c, ETH_DEV_SMI_WRITE_REG);

    xcore_freertos_periph_varlist_tx(
            c, 3,
            sizeof(phy_addr), &phy_addr,
            sizeof(reg_addr), &reg_addr,
            sizeof(val), &val);
}

uint16_t ethernet_driver_smi_read_reg(
        xcore_freertos_device_t dev,
        uint8_t phy_addr,
        uint8_t reg_addr)
{
    chanend c = xcore_freertos_dma_device_ctrl_chanend(dev);
    uint16_t retVal;

    xcore_freertos_periph_function_code_tx(c, ETH_DEV_SMI_READ_REG);

    xcore_freertos_periph_varlist_tx(
            c, 2,
            sizeof(phy_addr), &phy_addr,
            sizeof(reg_addr), &reg_addr);

    xcore_freertos_periph_varlist_rx(
            c, 1,
            sizeof(retVal), &retVal);

    return retVal;
}

ethernet_link_state_t ethernet_driver_smi_get_link_state(
        xcore_freertos_device_t dev,
        uint8_t phy_addr)
{
    chanend c = xcore_freertos_dma_device_ctrl_chanend(dev);
    ethernet_link_state_t retVal;

    xcore_freertos_periph_function_code_tx(c, ETH_DEV_SMI_GET_LINK_STATUS);

    xcore_freertos_periph_varlist_rx(
            c, 1,
            sizeof(retVal), &retVal);

    return retVal;
}

xcore_freertos_device_t ethernet_driver_init(
        int device_id,
        int rx_desc_count,
        int rx_buf_size,
        int tx_desc_count,
        void *app_data,
        int isr_core,
        rtos_irq_isr_t isr)
{
    xcore_freertos_device_t device;

    xassert(device_id >= 0 && device_id < BITSTREAM_ETHERNET_DEVICE_COUNT);

    device = bitstream_ethernet_devices[device_id];

    xcore_freertos_dma_device_common_init(
            device,
            rx_desc_count,
            rx_buf_size,
            tx_desc_count,
            app_data,
            isr_core,
            isr);

    return device;
}
