// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT WIFI_CONN_MGR

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* FreeRTOS Plus headers */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_Sockets.h"

/* Library headers */
#include "sl_wfx.h"
#include "sl_wfx_iot_wifi.h"
#include "wifi.h"
#include "heapsort.h"
#if USE_DHCPD
#include "dhcpd.h"
#endif

#define HWADDR_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define HWADDR_ARG(hwaddr) (hwaddr)[0], (hwaddr)[1], (hwaddr)[2], (hwaddr)[3], (hwaddr)[4], (hwaddr)[5]

#define MAX_SCAN_COUNT 20
#define MAX_CONNECTION_ATTEMPTS 3
#define MAX_CONNECTION_ATTEMPTS_BEFORE_CALLBACK 5

#define MAX_SCAN_ATTEMPTS 3

#define CACHED_AP_COUNT (2 * (MAX_SCAN_COUNT))
#define AP_CACHE_ENTRY_MAX_AGE 5

/*
 * If the signal stregth of the connected network drops
 * below this value, then start scanning to find a better AP.
 */
#define POOR_CONNECTION_RSSI_THRESHOLD -70

#define RSSI_HISTORY_LENGTH 16

#define NOTIFY_NETWORK_DOWN_BM          0x00000001
#define NOTIFY_WIFI_RESET_BM            0x00000002
#define NOTIFY_WIFI_PROFILES_UPDATED_BM 0x00000004

static TaskHandle_t wifi_conn_mgr_task_handle;

typedef struct {
    /*
     * 0 if this entry is inactive.
     * > 0 if it is active. Before each scan, it is incremented if it is > 0.
     * If the scan found this entry (bssids match) then it is set to 1.
     * If after the scan it is greater than AP_CACHE_ENTRY_MAX_AGE, it is set to 0.
     * If the scan finds a new AP that is not in the cache, this is set to 1.
     */
    int age;
    WIFINetworkProfile_t *associated_profile;

    int8_t rssi_hist[RSSI_HISTORY_LENGTH];
    int rssi;
    int rrsi_index;

    /*
     * Once this entry has been around long enough that the the rssi_hist table
     * is filled with unique values, this is set to 1. If there is an active
     * connection to another AP, then only switches to "stable" APs are considered.
     */
    int stable;

    uint8_t bssid[SL_WFX_BSSID_SIZE];
    int8_t channel;
    int8_t connect_failed;

} ap_info_t;

typedef struct {
    int profile_count;
    int ap_cache_count;
    sl_wfx_ssid_def_t ssid_scan_search_list[MAX_SCAN_COUNT];
    WIFINetworkProfile_t saved_profiles[MAX_SCAN_COUNT];
    ap_info_t cached_aps[CACHED_AP_COUNT];
    ap_info_t *sorted_aps[CACHED_AP_COUNT];
} scan_list_t;

void wifi_conn_mgr_network_event(eIPCallbackEvent_t eNetworkEvent)
{
    if (eNetworkEvent == eNetworkDown) {
        if (wifi_conn_mgr_task_handle != NULL) {
            rtos_printf("Network down, notifying the WiFi connection manager task\n");
            xTaskNotify(wifi_conn_mgr_task_handle, NOTIFY_NETWORK_DOWN_BM, eSetBits);
        }
    }
    /*
     * TODO: When eNetworkUp, call wifi_conn_mgr_event_cb() with a new WIFI_CONN_MGR_EVENT_NETWORK_UP?
     * This would let the app know that an IP has been assigned.
     * Alternatively, the app can provide vApplicationIPNetworkEventHook() and call wifi_conn_mgr_network_event().
     */
}

void sl_wfx_reset_request_callback(void)
{
    if (wifi_conn_mgr_task_handle != NULL) {
        rtos_printf("Requesting a WiFi module reset\n");
        xTaskNotify(wifi_conn_mgr_task_handle, NOTIFY_WIFI_RESET_BM, eSetBits);
    }
}

#if( ipconfigUSE_NETWORK_EVENT_HOOK != 1 )
    #error The WiFi connection manager requries that ipconfigUSE_NETWORK_EVENT_HOOK be set to 1
#endif
__attribute__((weak))
void vApplicationIPNetworkEventHook(eIPCallbackEvent_t eNetworkEvent)
{
    wifi_conn_mgr_network_event(eNetworkEvent);
}

static void build_scan_search_list(scan_list_t *scan_list)
{
    sl_wfx_ssid_def_t *list = scan_list->ssid_scan_search_list;
    WIFINetworkProfile_t *profile = scan_list->saved_profiles;

    int profile_count = 0;

    while (WIFI_NetworkGet(profile, profile_count) == eWiFiSuccess) {
        /* This is a correct usage of strncpy(). list->ssid does not need to be null terminiated. */
        strncpy((char *) list->ssid, profile->cSSID, SL_WFX_SSID_SIZE);
        list->ssid_length = strnlen((char *) list->ssid, SL_WFX_SSID_SIZE);

        profile_count++;
        list++;
        profile++;
        if (profile_count == MAX_SCAN_COUNT) {
            break;
        }
    }

    scan_list->profile_count = profile_count;
}

inline int ap_cmp(scan_list_t *scan_list, size_t a, size_t b)
{
    if (scan_list->sorted_aps[a]->age == 0) {
        return 1;
    } else if (scan_list->sorted_aps[b]->age == 0) {
        return -1;
    } else if (scan_list->sorted_aps[a]->rssi > scan_list->sorted_aps[b]->rssi) {
        return -1;
    } else {
        return 1;
    }
}

inline void ap_swap(scan_list_t *scan_list, size_t a, size_t b)
{
    ap_info_t *t;

    t = scan_list->sorted_aps[a];
    scan_list->sorted_aps[a] = scan_list->sorted_aps[b];
    scan_list->sorted_aps[b] = t;
}

/*
 * returns the index into the cache
 */
static int find_ap_in_cache(scan_list_t *scan_list, WIFIScanResult_t *scan_result)
{
    int i;
    ap_info_t *cached_ap;

    for (i = 0; i < scan_list->ap_cache_count; i++) {
        cached_ap = scan_list->sorted_aps[i];
        if (cached_ap->age > 0 && memcmp(scan_result->ucBSSID, cached_ap->bssid, SL_WFX_BSSID_SIZE) == 0) {
            return i;
        }
    }

    return -1;
}

static void update_ap_rssi(ap_info_t *ap, int8_t rssi)
{
    int i;

    ap->rssi_hist[ap->rrsi_index++] = rssi;
    if (ap->rrsi_index == RSSI_HISTORY_LENGTH) {
        ap->rrsi_index = 0;
        ap->stable = 1;
    }

    ap->rssi = 0;
    for (i = 0; i < RSSI_HISTORY_LENGTH; i++) {
        ap->rssi += ap->rssi_hist[i];
    }
    ap->rssi /= RSSI_HISTORY_LENGTH;
}

static void update_ap_in_cache(scan_list_t *scan_list, WIFIScanResult_t *scan_result, int cache_index)
{
    ap_info_t *cached_ap = scan_list->sorted_aps[cache_index];

    cached_ap->age = 1;
    cached_ap->channel = scan_result->cChannel;
    update_ap_rssi(cached_ap, scan_result->cRSSI);
}

static void add_ap_to_cache(scan_list_t *scan_list, WIFIScanResult_t *scan_result)
{
    int i;
    ap_info_t *cached_ap;

    for (i = 0; i < CACHED_AP_COUNT; i++) {
        cached_ap = scan_list->sorted_aps[i];
        if (cached_ap->age == 0) {
            break;
        }
    }
    if (i == CACHED_AP_COUNT) {
        return;
    }

    memcpy(cached_ap->bssid, scan_result->ucBSSID, SL_WFX_BSSID_SIZE);
    memset(cached_ap->rssi_hist, scan_result->cRSSI, sizeof(cached_ap->rssi_hist));
    cached_ap->rssi = scan_result->cRSSI;
    cached_ap->rrsi_index = 0;
    cached_ap->channel = scan_result->cChannel;
    cached_ap->connect_failed = 0;
    cached_ap->stable = 0;
    cached_ap->age = 1;

    cached_ap->associated_profile = NULL;
    for (i = 0; i < scan_list->profile_count; i++) {
        if (strncmp(scan_result->cSSID, scan_list->saved_profiles[i].cSSID, wificonfigMAX_SSID_LEN) == 0) {
            cached_ap->associated_profile = &scan_list->saved_profiles[i];
            break;
        }
    }
}

static int scan_networks(scan_list_t *scan_list)
{
    int ret = 0;
    int scan_count;
    int ap_cache_count;
    int i;
    WIFIScanResult_t scan_results[MAX_SCAN_COUNT];

    if (WIFI_Scan(scan_results, MAX_SCAN_COUNT) != eWiFiSuccess) {
        rtos_printf("WiFi scan failed\n");
        ret = -1;
    }

    for (i = 0; i < scan_list->ap_cache_count; i++) {

        /*
         * If any of the first ap_cache_count cached APs in the
         * sorted list have an age of 0, then something went wrong.
         * Since the following code makes this assumption, verify
         * it is true now.
         */
        xassert(scan_list->sorted_aps[i]->age > 0);

        if (scan_list->sorted_aps[i]->age > 0) {
            scan_list->sorted_aps[i]->age++;
        }
    }

    if (ret == 0) {
        WIFIScanResult_t *scan_result = scan_results;
        ap_info_t *ap;

        for (scan_count = 0; scan_count < MAX_SCAN_COUNT; scan_count++) {
            uint8_t no_bssid[wificonfigMAX_BSSID_LEN] = {0};

            if (memcmp(scan_result->ucBSSID, no_bssid, wificonfigMAX_BSSID_LEN) == 0) {
                break;
            }

            i = find_ap_in_cache(scan_list, scan_result);
            if (i != -1) {
                update_ap_in_cache(scan_list, scan_result, i);
            } else {
                add_ap_to_cache(scan_list, scan_result);
            }

            scan_result++;
        }

        for (i = 0; i < CACHED_AP_COUNT; i++) {
            ap = scan_list->sorted_aps[i];
            if (ap->age > AP_CACHE_ENTRY_MAX_AGE) {
                ap->age = 0;
            } else if (ap->age == 0) {
                break;
            }
        }

        rtos_printf("Scan found %d networks, sorting by signal strength\n", scan_count);

        heapsort(scan_list, i, ap_cmp, ap_swap);

        for (ap_cache_count = 0; ap_cache_count < CACHED_AP_COUNT; ap_cache_count++) {
            ap = scan_list->sorted_aps[ap_cache_count];
            if (ap->age > 0) {
                rtos_printf("%d: BSSID: " HWADDR_FMT "\n", ap_cache_count, HWADDR_ARG(ap->bssid));
                rtos_printf("\tChannel: %d\n", (int) ap->channel);
                rtos_printf("\tStrength: %d dBm\n", (int) ap->rssi);
                rtos_printf("\tAssociated profile: %s\n", ap->associated_profile != NULL ?
                                                          ap->associated_profile->cSSID : "NULL");
                rtos_printf("\tAge: %d\n", ap->age);
            } else {
                break;
            }
        }

        scan_list->ap_cache_count = ap_cache_count;
    }

    return ret;
}

static int connect_to_network(ap_info_t *ap)
{
    int ret = -1;
    int i;
    const WIFINetworkProfile_t *network_profile = ap->associated_profile;

    if (network_profile != NULL && ap->connect_failed < MAX_CONNECTION_ATTEMPTS) {
        WIFINetworkParams_t network_params;
        size_t tmp_len;
        WIFIReturnCode_t connected = eWiFiFailure;

        network_params.pcSSID = network_profile->cSSID;

        tmp_len = strnlen(network_profile->cSSID, wificonfigMAX_SSID_LEN);
        if (tmp_len < network_profile->ucSSIDLength) {
            network_params.ucSSIDLength = tmp_len;
        } else {
            network_params.ucSSIDLength = network_profile->ucSSIDLength;
        }

        network_params.pcPassword = network_profile->cPassword;

        tmp_len = strnlen(network_profile->cPassword, wificonfigMAX_PASSPHRASE_LEN);
        if (tmp_len < network_profile->ucPasswordLength) {
            network_params.ucPasswordLength = tmp_len;
        } else {
            network_params.ucPasswordLength = network_profile->ucPasswordLength;
        }

        network_params.xSecurity = network_profile->xSecurity;
        network_params.cChannel = ap->channel;

        for (i = 0; connected != eWiFiSuccess && i < MAX_CONNECTION_ATTEMPTS; i++) {
            WIFI_ConnectAPSetBSSID(ap->bssid);
            connected = WIFI_ConnectAP(&network_params);
            rtos_printf("WIFI_ConnectAP() returned %x\n", connected);
            if (connected != eWiFiSuccess) {
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }

        if (connected == eWiFiSuccess) {
            ap->connect_failed = 0;
            ret = 0;
        } else {
            if (ap->age == 1) {
                /*
                 * This AP is likely up since we just saw it in the scan.
                 * For some reason we can't connect to it though. Increment
                 * the connect failed count. Once this count is too high we
                 * will no longer try to connect to this AP.
                 */
                ap->connect_failed++;
            } else {
                /*
                 * The AP probably went offline. Set its age to the maximum,
                 * so that if it is not found on the next scan it will be
                 * removed from the cache and another connection attempt
                 * will not be made.
                 */
                ap->age = AP_CACHE_ENTRY_MAX_AGE;
            }
        }
    }

    return ret;
}

static void reset_scan_search_list_and_ap_cache(scan_list_t *scan_list)
{
    int i;

    rtos_printf("Loading WiFi profiles\n");

    memset(scan_list, 0, sizeof(scan_list_t));
    for (i = 0; i < CACHED_AP_COUNT; i++) {
        scan_list->sorted_aps[i] = &scan_list->cached_aps[i];
    }

    build_scan_search_list(scan_list);
    WIFI_ScanSetSSIDList(scan_list->ssid_scan_search_list, scan_list->profile_count);
}

static void wifi_conn_mgr_reload_profiles(void)
{
    if (wifi_conn_mgr_task_handle != NULL) {
        xTaskNotify(wifi_conn_mgr_task_handle, NOTIFY_WIFI_PROFILES_UPDATED_BM, eSetBits);
    }
}

int wifi_conn_mgr_disconnect_from_ap(int wifi_profiles_updated)
{
    int i = 0;

    if (wifi_profiles_updated) {
        wifi_conn_mgr_reload_profiles();
    }

    while (WIFI_Disconnect() != eWiFiSuccess) {
        vTaskDelay(pdMS_TO_TICKS(100));
        if (++i >= 3) {
            return -1;
        }
    }

    return 0;
}

int wifi_conn_mgr_stop_soft_ap(int wifi_profiles_updated)
{
    int i = 0;

    if (wifi_profiles_updated) {
        wifi_conn_mgr_reload_profiles();
    }

    while (WIFI_StopAP() != eWiFiSuccess) {
        vTaskDelay(pdMS_TO_TICKS(100));
        if (++i >= 3) {
            return -1;
        }
    }

    return 0;
}

__attribute__((weak))
int wifi_conn_mgr_event_cb(int state, char *soft_ap_ssid, char *soft_ap_password)
{
    (void) soft_ap_ssid;
    (void) soft_ap_password;

    return WIFI_CONN_MGR_MODE_STATION;
}

static inline int event_callback(int state, char *ssid_out, char *ssid_in, char *password_in)
{
    int mode;
    char tmp_ssid[wificonfigMAX_SSID_LEN + 1] = "";
    char tmp_password[wificonfigMAX_PASSPHRASE_LEN + 1] = "";

    if (ssid_out != NULL) {
        strncat(tmp_ssid, ssid_out, wificonfigMAX_SSID_LEN);
    }

    mode = wifi_conn_mgr_event_cb(state, tmp_ssid, tmp_password);

    if (mode == WIFI_CONN_MGR_MODE_SOFT_AP) {
        if (ssid_in != NULL) {
            ssid_in[0] = '\0';
            strncat(ssid_in, tmp_ssid, wificonfigMAX_SSID_LEN);
        }
        if (password_in != NULL) {
            password_in[0] = '\0';
            strncat(password_in, tmp_password, wificonfigMAX_PASSPHRASE_LEN);
        }
    }

    return mode;
}

static void wifi_conn_mgr(void *arg)
{
    scan_list_t scan_list;
    ap_info_t *connected_ap = NULL;
    int i;
    int failed_connection_attempts = 0;
    int mode = WIFI_CONN_MGR_MODE_STATION;
    WIFINetworkParams_t soft_ap_params;

    char soft_ap_ssid[wificonfigMAX_SSID_LEN + 1];
    char soft_ap_password[wificonfigMAX_PASSPHRASE_LEN + 1];

#if USE_DHCPD
    unsigned dhcpd_priority = (unsigned) arg;
#else
    (void) arg;
#endif

    while (WIFI_On() != eWiFiSuccess) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    reset_scan_search_list_and_ap_cache(&scan_list);

    /*
     * Wait for FreeRTOS IP task to initialize before attempting to
     * connect to an AP or starting a soft one
     */
    while (!xIPIsNetworkTaskReady()) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    mode = event_callback(WIFI_CONN_MGR_EVENT_STARTUP, NULL, soft_ap_ssid, soft_ap_password);

    for (;;) {

        uint32_t bits;
        ap_info_t *ap;
        int initiate_disconnect = 0;

        if (mode == WIFI_CONN_MGR_MODE_STATION) {

            int perform_connection = connected_ap == NULL;
            int poor_signal = !perform_connection && connected_ap->stable && connected_ap->rssi < POOR_CONNECTION_RSSI_THRESHOLD;

            if (perform_connection || poor_signal) {
                if (scan_list.profile_count > 0) {
                    int scan_attempts = 0;
                    while (scan_networks(&scan_list) != 0) {
                        scan_attempts++;
                        if (scan_attempts >= MAX_SCAN_ATTEMPTS) {
                            break;
                        }
                        vTaskDelay(pdMS_TO_TICKS(1000));
                    }
                }
            }

            if (poor_signal) {
                for (i = 0; i < scan_list.ap_cache_count; i++) {
                    ap = scan_list.sorted_aps[i];
                    if (ap != connected_ap && ap->connect_failed < MAX_CONNECTION_ATTEMPTS && ap->associated_profile != NULL && ap->stable && ap->rssi > connected_ap->rssi + 10) {
                        rtos_printf("%s:%d is a better AP, connect to it\n", ap->associated_profile->cSSID, ap->channel);
                        perform_connection = 1;
                        initiate_disconnect = 1;
                        connected_ap = NULL;
                        break;
                    } else {
                        if (ap->rssi <= connected_ap->rssi) {
                            break;
                        }
                    }
                }
            }

            if (perform_connection) {
                for (i = 0; i < scan_list.ap_cache_count; i++) {
                    ap = scan_list.sorted_aps[i];
                    rtos_printf("ATTEMPTING NEW CONNECTION\n");
                    if (connect_to_network(ap) != -1) {
                        connected_ap = ap;
                        break;
                    }
                }

                if (connected_ap == NULL) {
                    failed_connection_attempts++;
                    if (failed_connection_attempts == MAX_CONNECTION_ATTEMPTS_BEFORE_CALLBACK || scan_list.profile_count == 0) {
                        failed_connection_attempts = 0;
                        mode = event_callback(WIFI_CONN_MGR_EVENT_CONNECT_FAILED, NULL, soft_ap_ssid, soft_ap_password);
                    }
                } else {
                    failed_connection_attempts = 0;
                    event_callback(WIFI_CONN_MGR_EVENT_CONNECTED, connected_ap->associated_profile->cSSID, NULL, NULL);
                }
            } else {
                /* Already connected to an AP, connected_ap is not NULL */

                /*
                 * TODO: maybe still perform a scan periodically to keep the cache
                 * up to date, just less frequently?
                 */
                uint32_t rcpi;

                if (sl_wfx_get_signal_strength(&rcpi) == SL_STATUS_OK) {
                    int8_t rssi = ((int) rcpi - 220) / 2;
                    update_ap_rssi(connected_ap, rssi);
                    //rtos_printf("Current signal strength of " HWADDR_FMT " is %d dBm\n", HWADDR_ARG(connected_ap->bssid), connected_ap->rssi);
                }
            }
        }

        if (mode == WIFI_CONN_MGR_MODE_SOFT_AP) {

            if (!WIFI_IsConnected()) {

                soft_ap_params.pcSSID = soft_ap_ssid;
                soft_ap_params.pcPassword = soft_ap_password;
                soft_ap_params.ucSSIDLength = strnlen(soft_ap_ssid, wificonfigMAX_SSID_LEN);
                soft_ap_params.ucPasswordLength = strnlen(soft_ap_password, wificonfigMAX_PASSPHRASE_LEN);

                /*
                 * TODO: perhaps implement an algorithm to scan and then choose
                 * the best channel between 1, 6, and 11.
                 */
                soft_ap_params.cChannel = 6;

                if (soft_ap_params.ucPasswordLength > 0) {
                    soft_ap_params.xSecurity = eWiFiSecurityWPA2;
                } else {
                    soft_ap_params.xSecurity = eWiFiSecurityOpen;
                }

                WIFI_ConfigureAP(&soft_ap_params);
                while (WIFI_StartAP() != eWiFiSuccess) {
                    rtos_printf("WIFI_StartAP() failed, will try again\n");
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
#if USE_DHCPD
                /* note: It's safe to call this if the DHCP server is already started. */
                dhcpd_start(dhcpd_priority);
#endif

                event_callback(WIFI_CONN_MGR_EVENT_SOFT_AP_STARTED, soft_ap_ssid, NULL, NULL);
            }
        }

        /*
         * TODO: Get rid of the magic number timeout
         */

        if (xTaskNotifyWait(0x00000000,
                            0xFFFFFFFF,
                            &bits,
                            pdMS_TO_TICKS(5000)) == pdTRUE) {

            if (bits & NOTIFY_WIFI_RESET_BM) {
                rtos_printf("Resetting the WiFi module\n");
                while (WIFI_Reset() != eWiFiSuccess) {
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
                connected_ap = NULL;
            } else {
                if (bits & NOTIFY_WIFI_PROFILES_UPDATED_BM) {
                    reset_scan_search_list_and_ap_cache(&scan_list);
                }
                if (bits & NOTIFY_NETWORK_DOWN_BM) {
                    if (mode == WIFI_CONN_MGR_MODE_STATION) {
                        if (WIFI_IsConnected()) {
                            /*
                             * It is likely from the disconnect event
                             * before a successful reconnection.
                             */
                            if (!initiate_disconnect) {
                                rtos_printf("Unexpected superfluous WiFi disconnect event, ignoring\n");
                            } else {
                                rtos_printf("Superfluous WiFi disconnect event, ignoring\n");
                            }
                        } else {
                            if (initiate_disconnect) {
                                /* A reconnection failed. Wait before retrying. */
                                rtos_printf("Reconnection failed? Will wait before retrying.\n");
                                vTaskDelay(pdMS_TO_TICKS(5000));
                            } else {
                                mode = event_callback(WIFI_CONN_MGR_EVENT_DISCONNECTED, connected_ap != NULL ? connected_ap->associated_profile->cSSID : NULL, soft_ap_ssid, soft_ap_password);
                            }

                            /*
                             * Ensure the connected AP is NULL so
                             * that a reconnection is attempted.
                             */
                            connected_ap = NULL;
                        }
                    } else if (mode == WIFI_CONN_MGR_MODE_SOFT_AP) {
                        if (!WIFI_IsConnected()) {
                            mode = event_callback(WIFI_CONN_MGR_EVENT_SOFT_AP_STOPPED, soft_ap_ssid, soft_ap_ssid, soft_ap_password);
                            if (mode != WIFI_CONN_MGR_MODE_SOFT_AP) {
#if USE_DHCPD
                                dhcpd_stop();
#endif

                                /*
                                 * Ensure the connected AP is NULL so
                                 * that a reconnection is attempted.
                                 */
                                connected_ap = NULL;
                            }
                        } else {
                            rtos_printf("Unexpected superfluous WiFi disconnect event, ignoring\n");
                        }
                    }
                }
            }
        }
    }
}

void wifi_conn_mgr_start(unsigned manager_priority, unsigned dhcpd_priority)
{
    xTaskCreate(
            (TaskFunction_t) wifi_conn_mgr,
            "wifi_conn_mgr",
            portTASK_STACK_DEPTH(wifi_conn_mgr),
            (void *) dhcpd_priority,
            manager_priority,
            &wifi_conn_mgr_task_handle);
}
