// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef DHCPD_H_
#define DHCPD_H_

#define DHCPD_TASK_NAME "dhcpd"
#define DHCPD_TASK_STACKSIZE (portTASK_STACK_DEPTH(dhcpd_task))

#define DHPCD_SERVER_IP         FreeRTOS_inet_addr_quick(10,0,0,1)
#define DHPCD_IP_POOL_START     FreeRTOS_inet_addr_quick(10,0,0,0)
#define DHPCD_IP_POOL_END       FreeRTOS_inet_addr_quick(10,0,0,10)
#define DHPCD_NETMASK           FreeRTOS_inet_addr_quick(255,0,0,0)
#define DHPCD_GATEWAY           FreeRTOS_inet_addr_quick(10,0,0,1)
#define DHPCD_PRIMARY_DNS       FreeRTOS_inet_addr_quick(10,0,0,5)
#define DHPCD_SECONDARY_DNS     FreeRTOS_inet_addr_quick(10,0,0,6)


/**
 * How long in seconds IP leases are for.
 */
#define DHCPD_LEASE_TIME 120

/**
 * The amount of time in seconds an IP address is
 * reserved for after sending an offer to a client.
 * If a request is not received within this
 * amount of time the IP will be made available
 * again to new clients.
 */
#define DHCPD_OFFER_EXPIRATION_TIME 10

/**
 * How often in seconds to wake up and check to see if
 * leases have expired and if unavailable IP addresses
 * should be probed.
 */
#define DHCPD_REFRESH_INTERVAL      5

/**
 * Set to 1 to probe newly allocated IP addresses
 * for availability. When this is enabled, there
 * will be a delay of at least
 * DHCPD_IP_PROBE_ATTEMPTS*DHCPD_IP_PROBE_WAIT_TIME
 * milliseconds between the DISCOVER and OFFER messages
 * if the offered IP has not previously been assigned to
 * the client.
 *
 * Set to 0 to disable initial probing.
 */
#define DHCPD_PROBE_NEW_IP_ADDRESSES      1

/**
 * How often in seconds to probe IP addresses that
 * are marked as unavailable to determine if they
 * are still unavailable.
 *
 * Set to 0 to disable periodic probing.
 */
#define DHCPD_UNAVAILABLE_IP_PROBE_INTERVAL 10

/**
 * The number of probes to send when checking to
 * see if an IP is in use.
 */
#define DHCPD_IP_PROBE_ATTEMPTS 2

/**
 * How long in milliseconds to wait between probes
 * when checking to see if an IP is in use.
 */
#define DHCPD_IP_PROBE_WAIT_TIME 250


void dhcpd_start(UBaseType_t priority);
void dhcpd_stop();


#endif /* DHCPD_H_ */
