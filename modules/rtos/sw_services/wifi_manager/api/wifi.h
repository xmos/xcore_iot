// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef WIFI_H_
#define WIFI_H_

#define WIFI_CONN_MGR_MODE_STATION 0
#define WIFI_CONN_MGR_MODE_SOFT_AP 1

#define WIFI_CONN_MGR_EVENT_STARTUP         0
#define WIFI_CONN_MGR_EVENT_CONNECT_FAILED  1
#define WIFI_CONN_MGR_EVENT_CONNECTED       2
#define WIFI_CONN_MGR_EVENT_DISCONNECTED    3
#define WIFI_CONN_MGR_EVENT_SOFT_AP_STARTED 4
#define WIFI_CONN_MGR_EVENT_SOFT_AP_STOPPED 5

/**
 * The application may provide this function. It is called once by the WiFi
 * connection manager task during various events. The application may use this
 * to direct the WiFi manager task to transition into either Soft AP or station
 * mode.
 *
 * The following describes when the different values are passed to \p event.
 *
 * - WIFI_CONN_MGR_EVENT_STARTUP Called when the WiFi connection manager task first
 * starts. The application might decide to go directly into soft AP mode at this point,
 * for example if the user is holding down a certain button, or if it knows this is the
 * first bootup after a reset and there are no saved WiFi profiles.
 *
 * - WIFI_CONN_MGR_EVENT_CONNECT_FAILED Called after the WiFi connection manager task
 * fails to connect to any AP several times in a row. This might be a good time for
 * the application to decide to transition to soft AP mode.
 *
 * - WIFI_CONN_MGR_EVENT_CONNECTED Called after a successful connection to an AP. The
 * return value is ignored and the application cannot change the mode at this point.
 * \p ssid points to the SSID of the network that was joined. \p password is unused.
 *
 * - WIFI_CONN_MGR_EVENT_DISCONNECTED Called after being disconnected from an AP.
 * This can happen for many different reasons. If the application requested the
 * disconnection by calling wifi_conn_mgr_disconnect_from_ap(), then it might want to
 * return WIFI_CONN_MGR_MODE_SOFT_AP to transition to soft AP mode. Otherwise it should
 * return WIFI_CONN_MGR_MODE_STATION so that the connection manager task can attempt to
 * reconnect. \p ssid usually points to the SSID of the network that got disconnected,
 * but under certain circumstances it might be zero length.
 *
 * - WIFI_CONN_MGR_EVENT_SOFT_AP_STARTED Called after successfully starting a soft AP.
 * The return value is ignored and the application cannot change the mode at this point.
 * \p ssid points to the SSID of the soft AP that was started. \p password is unused.
 *
 * - WIFI_CONN_MGR_EVENT_SOFT_AP_STOPPED Called after stopping the soft AP. \p ssid
 * points to the SSID of the soft AP that was stopped. It is almost certain that this is
 * due to the application calling wifi_conn_mgr_stop_soft_ap(), and so the application
 * might want to return WIFI_CONN_MGR_MODE_STATION to transition to station mode so that
 * the WiFi manager can start trying to connect to an AP. Otherwise, WIFI_CONN_MGR_MODE_SOFT_AP
 * may be returned to stay in soft AP mode, perhaps changing either the SSID or the password.
 *
 * With the exception of the WIFI_CONN_MGR_EVENT_CONNECTED and WIFI_CONN_MGR_EVENT_SOFT_AP_STARTED
 * events, the value returned sets the WiFi mode, and must be either WIFI_CONN_MGR_MODE_STATION
 * or WIFI_CONN_MGR_MODE_SOFT_AP. When the returned mode is not ignored and when returning
 * WIFI_CONN_MGR_MODE_SOFT_AP, the SSID and password to use for the soft AP must be copied
 * into the \p ssid and \p password pointer parameters. When returning WIFI_CONN_MGR_MODE_STATION
 * the contents of \p ssid and \p password are ignored.
 *
 * If the application does not implement this function, then the WiFi manager
 * will always be in station mode and won't ever start a soft AP.
 *
 * \param[in,out] ssid The SSID of the AP associated with the event when the
 *                event is WIFI_CONN_MGR_EVENT_CONNECTED, WIFI_CONN_MGR_EVENT_DISCONNECTED,
 *                WIFI_CONN_MGR_EVENT_SOFT_AP_STARTED, or WIFI_CONN_MGR_EVENT_SOFT_AP_STOPPED.
 *                The SSID of the soft AP must also be copied here when returning the mode
 *                WIFI_CONN_MGR_MODE_SOFT_AP.
 *
 * \param[in,out] password The password of the soft AP must be copied here when returning
 *                the mode WIFI_CONN_MGR_MODE_SOFT_AP. If the password has a length of
 *                zero then the soft AP will be open. Otherwise it will use WPA2.
 *
 * \retval WIFI_CONN_MGR_MODE_STATION This tells the manager to scan for and
 *         attempt to connect to an AP. Any strings copied into either the
 *         \p ssid or \p password parameters are ignored.
 *
 * \retval WIFI_CONN_MGR_MODE_SOFT_AP This tells the manager to start a soft AP.
 *         The SSID and password to use for the soft AP must be copied into the
 *         \p ssid and \p password respectively.
 *
 */
int wifi_conn_mgr_event_cb(int event, char *ssid, char *password);

/**
 * Should be called by the application when it wants to disconnect from an
 * AP. This could be because the WiFi profiles have been updated, or if it
 * wants to start a soft AP.
 *
 * Note that after the disconnection is complete, the callback function
 * wifi_conn_mgr_event_cb() will be called with the
 * WIFI_CONN_MGR_EVENT_DISCONNECTED state. If the application wants to start
 * a soft AP it must return WIFI_CONN_MGR_MODE_SOFT_AP. If it wants to stay
 * in station mode then it must return WIFI_CONN_MGR_MODE_STATION.
 *
 * \param wifi_profiles_updated Set to true if the WiFi profiles were updated.
 *                              This tells the wifi connection manager to reload
 *                              the profiles.
 *
 * \retval  0 if the disconnection is successful.
 * \retval -1 if the disconnection fails.
 */
int wifi_conn_mgr_disconnect_from_ap(int wifi_profiles_updated);


/**
 * Should be called by the application when it is ready to stop the soft AP
 * and transition to station mode.
 *
 * Note that after the AP is successfully stopped the callback function
 * wifi_conn_mgr_event_cb() will be called with the
 * WIFI_CONN_MGR_EVENT_SOFT_AP_STOPPED state, and the application must
 * return WIFI_CONN_MGR_MODE_STATION to actually move into station mode.
 *
 * \param wifi_profiles_updated Set to true if the WiFi profiles were updated
 *                              while in soft AP mode. This tells the
 *                              wifi connection manager to reload the profiles.
 *
 * \retval  0 if the soft AP is successfully stopped.
 * \retval -1 if the soft AP fails to stop.
 */
int wifi_conn_mgr_stop_soft_ap(int wifi_profiles_updated);

/**
 * Starts the WiFi connection manager task. This handles automatically connecting
 * to WiFi networks that have been saved to the filesystem, for example with the
 * WIFI_NetworkAdd() function, and are retrievable with the WIFI_NetworkGet()
 * function.
 *
 * It can also start a soft AP if requested by the application.
 *
 * \param manager_priority The priority to use for the WiFi manager task.
 * \param dhcpd_priority   The priority to use for the DHCP server, if it
 *                         is started.
 */
void wifi_conn_mgr_start(unsigned manager_priority, unsigned dhcpd_priority);

#endif /* WIFI_H_ */
