// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef ETHERNET_DRIVER_H_
#define ETHERNET_DRIVER_H_

#include "soc.h"
#include "eth_dev_ctrl.h"

/** Type representing link events. */
typedef enum ethernet_link_state_t {
  ETHERNET_LINK_DOWN,    /**< Ethernet link down event. */
  ETHERNET_LINK_UP       /**< Ethernet link up event. */
} ethernet_link_state_t;

#define ETHERNET_MACADDR_NUM_BYTES 6

void ethernet_driver_send_packet(
        soc_peripheral_t dev,
        void *packet,
        unsigned n);

void ethernet_driver_send_packet_wait_for_copy(
        soc_peripheral_t dev,
        void *packet,
        unsigned n);

void ethernet_get_mac_addr(
        soc_peripheral_t dev,
        size_t ifnum,
        uint8_t mac_address[ETHERNET_MACADDR_NUM_BYTES]);

void ethernet_set_mac_addr(
        soc_peripheral_t dev,
        size_t ifnum,
        const uint8_t mac_address[ETHERNET_MACADDR_NUM_BYTES]);

void ethernet_driver_smi_write_reg(
        soc_peripheral_t dev,
        uint8_t phy_addr,
        uint8_t reg_addr,
        uint16_t val);

uint16_t ethernet_driver_smi_read_reg(
        soc_peripheral_t dev,
        uint8_t phy_addr,
        uint8_t reg_addr);

ethernet_link_state_t ethernet_driver_smi_get_link_state(
        soc_peripheral_t dev,
        uint8_t phy_addr);

soc_peripheral_t ethernet_driver_init(
        int device_id,
        int rx_desc_count,
        int rx_buf_size,
        int tx_desc_count,
        void *app_data,
        int isr_core,
        rtos_irq_isr_t isr);

#endif /* ETHERNET_DRIVER_H_ */
