// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef ETH_DEV_H_
#define ETH_DEV_H_

#if __rtos_peripherals_conf_h_exists__
#include "rtos_peripherals_conf.h"
#endif

#include "eth_dev_conf_defaults.h"
#include "eth_dev_ctrl.h"

#include "ethernet.h"
#include "smi.h"
#include "otp_board_info.h"

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
        otp_ports_t &?otp_ports);

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
        otp_ports_t &?otp_ports);


#endif /* ETH_DEV_H_ */
