// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef SL_WFX_HOST_H_
#define SL_WFX_HOST_H_

#include "FreeRTOS.h"
#include "event_groups.h"

#include "soc.h"
#include "gpio_driver.h"

#define SL_WFX_HIF_GPIO_WUP   0
#define SL_WFX_HIF_GPIO_RESET 1

/*
 * This is the number of padding bytes found before
 * most Ethernet frames. Extra memory movement needs
 * to take place for frames that have a different
 * number of padding bytes.
 */
#define SL_WFX_NORMAL_FRAME_PAD_LENGTH 2

/* Wi-Fi events*/
#define SL_WFX_INTERRUPT     ( 1 << 0 )
#define SL_WFX_CONNECT       ( 1 << 1 )
#define SL_WFX_DISCONNECT    ( 1 << 2 )
#define SL_WFX_START_AP      ( 1 << 3 )
#define SL_WFX_STOP_AP       ( 1 << 4 )
#define SL_WFX_SCAN_COMPLETE ( 1 << 5 )
#define SL_WFX_CONNECT_FAIL  ( 1 << 6 )
#define SL_WFX_START_AP_FAIL ( 1 << 7 )
#define SL_WFX_INITIALIZED   ( 1 << 8 )

extern EventGroupHandle_t sl_wfx_event_group;

typedef struct __attribute__((__packed__)) scan_result_list_s {
  sl_wfx_ssid_def_t ssid_def;
  uint8_t  mac[SL_WFX_MAC_ADDR_SIZE];
  uint16_t channel;
  sl_wfx_security_mode_bitmask_t security_mode;
  uint16_t rcpi;
} scan_result_list_t;

/*
 * Must be called prior to calling sl_wfx_init() to let the driver
 * know which devices to use for SPI and GPIO, as well as which
 * GPIO ports and pins to use for the WIRQ, WUP, and RESET pins.
 *
 * The GPIO device must already be initialized.
 * The SPI device must not already be initialized, as the wfx200
 * driver will initialize it.
 */
void sl_wfx_host_set_hif(int spi_dev_id,
		                 int gpio_dev_id,
                         gpio_id_t wirq_gpio_port, int wirq_bit,
                         gpio_id_t wup_gpio_port, int wup_bit,
                         gpio_id_t reset_gpio_port, int reset_bit);

/*
 * Must be called prior to calling sl_wfx_init() to let the driver
 * know where the PDS data is stored.
 */
void sl_wfx_host_set_pds(const char * const pds_data[],
                         uint16_t pds_size);

/*
 * This is called by the host driver code when unrecoverable errors
 * are detected. It deinitializes the host interface bus and holds
 * the WFX module in reset, without attempting to gracefully shut it
 * down - no communication with the module is attempted. The application
 * is also notified via a callback that the module has been reset so
 * that it may attempt to bring it back up.
 */
void sl_wfx_host_reset(void);

/*
 * Callback functions to be implemented by the application
 */
void sl_wfx_scan_result_callback(sl_wfx_scan_result_ind_body_t* scan_result);
void sl_wfx_scan_complete_callback(sl_wfx_fmac_status_t status);
void sl_wfx_connect_callback(uint8_t* mac, sl_wfx_fmac_status_t status);
void sl_wfx_disconnect_callback(uint8_t* mac, sl_wfx_disconnected_reason_t reason);
void sl_wfx_start_ap_callback(sl_wfx_fmac_status_t status);
void sl_wfx_stop_ap_callback(void);
void sl_wfx_client_connected_callback(uint8_t* mac);
void sl_wfx_ap_client_rejected_callback(sl_wfx_reason_t reason, uint8_t* mac);
void sl_wfx_ap_client_disconnected_callback(sl_wfx_reason_t reason, uint8_t* mac);
void sl_wfx_host_received_frame_callback(sl_wfx_received_ind_t* rx_buffer);
void sl_wfx_reset_request_callback(void);

#endif /* SL_WFX_HOST_H_ */
