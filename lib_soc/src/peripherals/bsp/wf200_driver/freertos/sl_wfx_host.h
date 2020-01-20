// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef SL_WFX_HOST_H_
#define SL_WFX_HOST_H_

#include "FreeRTOS.h"
#include "event_groups.h"

#include "soc.h"
#include "gpio_driver.h"

#define SL_WFX_EVENT_MAX_SIZE   512
#define SL_WFX_EVENT_LIST_SIZE  1
#define SL_WFX_MAX_STATIONS     8
#define SL_WFX_MAX_SCAN_RESULTS 50

#define SL_WFX_HIF_GPIO_WUP   0
#define SL_WFX_HIF_GPIO_RESET 1

/* Wi-Fi events*/
#define SL_WFX_INTERRUPT     ( 1 << 0 )
#define SL_WFX_CONNECT           ( 1 << 1 )
#define SL_WFX_DISCONNECT    ( 1 << 2 )
#define SL_WFX_START_AP          ( 1 << 3 )
#define SL_WFX_STOP_AP           ( 1 << 4 )
#define SL_WFX_SCAN_COMPLETE     ( 1 << 5 )

extern EventGroupHandle_t sl_wfx_event_group;

typedef struct __attribute__((__packed__)) scan_result_list_s {
  sl_wfx_ssid_def_t ssid_def;
  uint8_t  mac[SL_WFX_MAC_ADDR_SIZE];
  uint16_t channel;
  sl_wfx_security_mode_bitmask_t security_mode;
  uint16_t rcpi;
} scan_result_list_t;

void sl_wfx_host_set_hif(soc_peripheral_t spi_dev,
                         soc_peripheral_t gpio_dev,
                         gpio_id_t wirq_gpio_port, int wirq_bit,
                         gpio_id_t wup_gpio_port, int wup_bit,
                         gpio_id_t reset_gpio_port, int reset_bit);

void sl_wfx_host_gpio(int gpio,
                      int value);

void sl_wfx_host_set_pds(const char *pds_data[],
                         uint16_t pds_size);

#endif /* SL_WFX_HOST_H_ */
