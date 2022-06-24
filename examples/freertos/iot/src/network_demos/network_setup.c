// Copyright 2019-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
/* System headers */
#include <platform.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "task.h"

/* Library headers */
#include "ff.h"
#include "sl_wfx_iot_wifi.h"
#include "wifi.h"

/* App headers */
#include "app_conf.h"
#include "network_setup.h"

#define HWADDR_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define HWADDR_ARG(hwaddr) \
  (hwaddr)[0], (hwaddr)[1], (hwaddr)[2], (hwaddr)[3], (hwaddr)[4], (hwaddr)[5]

const uint8_t ucIPAddress[ipIP_ADDRESS_LENGTH_BYTES] = {
    (uint8_t)IPconfig_IP_ADDR_OCTET_0, (uint8_t)IPconfig_IP_ADDR_OCTET_1,
    (uint8_t)IPconfig_IP_ADDR_OCTET_2, (uint8_t)IPconfig_IP_ADDR_OCTET_3};

const uint8_t ucNetMask[ipIP_ADDRESS_LENGTH_BYTES] = {
    (uint8_t)IPconfig_NET_MASK_OCTET_0, (uint8_t)IPconfig_NET_MASK_OCTET_1,
    (uint8_t)IPconfig_NET_MASK_OCTET_2, (uint8_t)IPconfig_NET_MASK_OCTET_3};

const uint8_t ucGatewayAddress[ipIP_ADDRESS_LENGTH_BYTES] = {
    (uint8_t)IPconfig_GATEWAY_OCTET_0, (uint8_t)IPconfig_GATEWAY_OCTET_1,
    (uint8_t)IPconfig_GATEWAY_OCTET_2, (uint8_t)IPconfig_GATEWAY_OCTET_3};

const uint8_t ucDNSServerAddress[ipIP_ADDRESS_LENGTH_BYTES] = {
    (uint8_t)IPconfig_DNS_SERVER_OCTET_0, (uint8_t)IPconfig_DNS_SERVER_OCTET_1,
    (uint8_t)IPconfig_DNS_SERVER_OCTET_2, (uint8_t)IPconfig_DNS_SERVER_OCTET_3};

const uint8_t ucMACAddress[ipMAC_ADDRESS_LENGTH_BYTES] = {
    (uint8_t)IPconfig_MAC_ADDR_OCTET_0, (uint8_t)IPconfig_MAC_ADDR_OCTET_1,
    (uint8_t)IPconfig_MAC_ADDR_OCTET_2, (uint8_t)IPconfig_MAC_ADDR_OCTET_3,
    (uint8_t)IPconfig_MAC_ADDR_OCTET_4, (uint8_t)IPconfig_MAC_ADDR_OCTET_5};

static void scan(void) {
  WIFIScanResult_t scan_results[10];

  if (sl_wfx_set_scan_parameters(550, 550, 2) == SL_STATUS_OK) {
    rtos_printf("Scan parameters set\n");
  } else {
    rtos_printf("Failed to set scan parameters\n");
  }

  if (WIFI_Scan(scan_results, 10) == eWiFiSuccess) {
    WIFIScanResult_t *scan_result = scan_results;

    for (int i = 0; i < 10; i++) {
      uint8_t no_bssid[wificonfigMAX_BSSID_LEN] = {0};

      if (memcmp(scan_result->ucBSSID, no_bssid, wificonfigMAX_BSSID_LEN) ==
          0) {
        break;
      }

      rtos_printf("Scan result %d:\n", i);
      rtos_printf("\tBSSID: " HWADDR_FMT "\n", i,
                  HWADDR_ARG(scan_result->ucBSSID));
      rtos_printf("\tSSID: %s\n", scan_result->cSSID);
      rtos_printf(
          "\tSecurity: %s\n",
          scan_result->xSecurity == eWiFiSecurityOpen
              ? "open"
              : scan_result->xSecurity == eWiFiSecurityWEP ? "WEP" : "WPA");
      rtos_printf("\tChannel: %d\n", (int)scan_result->cChannel);
      rtos_printf("\tStrength: %d dBm\n\n", (int)scan_result->cRSSI);

      scan_result++;
    }
  }
}

int wifi_conn_mgr_event_cb(int event, char *ssid, char *password) {
  switch (event) {
    case WIFI_CONN_MGR_EVENT_STARTUP:
      rtos_printf("Directing WiFi manager to go into station mode\n");
      return WIFI_CONN_MGR_MODE_STATION;

    case WIFI_CONN_MGR_EVENT_CONNECT_FAILED:

      scan();

      rtos_printf("Directing WiFi manager to start a soft AP\n");
      rtos_printf("\tSSID is %s\n", appconfSOFT_AP_SSID);
      if (strlen(appconfSOFT_AP_PASSWORD) > 0) {
        rtos_printf("\tPassword is %s\n");
      } else {
        rtos_printf("\tThere is no password\n");
      }
      strcpy(ssid, appconfSOFT_AP_SSID);
      strcpy(password, appconfSOFT_AP_PASSWORD);
      return WIFI_CONN_MGR_MODE_SOFT_AP;

    case WIFI_CONN_MGR_EVENT_CONNECTED: {
      rtos_printf("Connected to %s\n", ssid);

      return WIFI_CONN_MGR_MODE_STATION; /* this is ignored */
    }
    case WIFI_CONN_MGR_EVENT_DISCONNECTED:
      if (ssid[0] != '\0') {
        rtos_printf("Disconnected from %s\n", ssid);
      } else {
        rtos_printf("Disconnected from AP\n");
      }

      return WIFI_CONN_MGR_MODE_STATION;

    case WIFI_CONN_MGR_EVENT_SOFT_AP_STARTED:
      rtos_printf("Soft AP %s started\n", ssid);
      return WIFI_CONN_MGR_MODE_SOFT_AP; /* this is ignored */

    case WIFI_CONN_MGR_EVENT_SOFT_AP_STOPPED:
      rtos_printf("Soft AP %s stopped. Going into station mode\n", ssid);
      return WIFI_CONN_MGR_MODE_STATION;

    default:
      return WIFI_CONN_MGR_MODE_STATION;
  }
}

static FIL wf200_fw_file;
static uint32_t wf200_fw_size;

uint32_t sl_wfx_app_fw_size(void) {
  FRESULT result = FR_OK;

  if (wf200_fw_file.obj.fs == NULL) {
    rtos_printf("Opening WF200 firmware file\n");
    result = f_open(&wf200_fw_file, "/flash/firmware/wf200.sec", FA_READ);
  }

  if (result == FR_OK) {
    wf200_fw_size = f_size(&wf200_fw_file);
  } else {
    wf200_fw_size = 0;
  }

  rtos_printf("wf200 fw size is %d\n", wf200_fw_size);
  return wf200_fw_size;
}

sl_status_t sl_wfx_app_fw_read(uint8_t *data, uint32_t index, uint32_t size) {
  FRESULT result;
  UINT bytes_read = 0;

  if (wf200_fw_file.obj.fs != NULL) {
    result = f_read(&wf200_fw_file, data, size, &bytes_read);
  }

  if (bytes_read == 0 || index + size >= wf200_fw_size) {
    if (wf200_fw_file.obj.fs != NULL) {
      f_close(&wf200_fw_file);
      wf200_fw_size = 0;
      rtos_printf("Closed WF200 firmware file\n");
    }
  }

  if (bytes_read == size) {
    return SL_STATUS_OK;
  } else {
    rtos_printf("items_read: %d\n", bytes_read);
    return SL_STATUS_FAIL;
  }
}

typedef struct {
  rtos_spi_master_device_t *wifi_device_ctx;
  rtos_gpio_t *gpio_ctx;
} wifi_devices_t;

static void wifi_setup_task(wifi_devices_t *wifi_devs) {
  const rtos_gpio_port_id_t wifi_wup_rst_port = rtos_gpio_port(WIFI_WUP_RST_N);
  const rtos_gpio_port_id_t wifi_irq_port = rtos_gpio_port(WIFI_WIRQ);

  rtos_spi_master_device_t *wifi_device_ctx = wifi_devs->wifi_device_ctx;
  rtos_gpio_t *gpio_ctx = wifi_devs->gpio_ctx;

  vTaskDelay(pdMS_TO_TICKS(100));

  sl_wfx_host_set_hif(wifi_device_ctx, gpio_ctx, wifi_irq_port, 0,
                      wifi_wup_rst_port, 0, wifi_wup_rst_port, 1);

  rtos_printf("Start FreeRTOS_IP\n");
  FreeRTOS_IPInit(ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress,
                  ucMACAddress);

  rtos_printf("Start WiFi connection manager\n");
  wifi_conn_mgr_start(appconfWIFI_CONN_MNGR_TASK_PRIORITY,
                      appconfWIFI_DHCP_SERVER_TASK_PRIORITY);

  vTaskDelete(NULL);
}

void wifi_start(rtos_spi_master_device_t *wifi_device_ctx,
                rtos_gpio_t *gpio_ctx) {
  static wifi_devices_t wifi_devs;

  wifi_devs.wifi_device_ctx = wifi_device_ctx;
  wifi_devs.gpio_ctx = gpio_ctx;

  xTaskCreate((TaskFunction_t)wifi_setup_task, "wifi_setup_task",
              RTOS_THREAD_STACK_SIZE(wifi_setup_task), &wifi_devs,
              appconfWIFI_SETUP_TASK_PRIORITY, NULL);
}
