// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef DHCPD_H_
#define DHCPD_H_

#define DHCPD_TASK_NAME "dhcpd"

#define DHCPD_OCTETS_TO_IP_ADDR(O0, O1, O2, O3) \
    ((((uint32_t)(O3)) << 24) |                 \
     (((uint32_t)(O2)) << 16) |                 \
     (((uint32_t)(O1)) <<  8) |                 \
     (((uint32_t)(O0)) <<  0))                  \

#ifndef DHPCD_SERVER_IP
#define DHPCD_SERVER_IP         DHCPD_OCTETS_TO_IP_ADDR(10,0,0,1)
#endif
#ifndef DHPCD_IP_POOL_START
#define DHPCD_IP_POOL_START     DHCPD_OCTETS_TO_IP_ADDR(10,0,0,0)
#endif
#ifndef DHPCD_IP_POOL_END
#define DHPCD_IP_POOL_END       DHCPD_OCTETS_TO_IP_ADDR(10,0,0,10)
#endif
#ifndef DHPCD_NETMASK
#define DHPCD_NETMASK           DHCPD_OCTETS_TO_IP_ADDR(255,0,0,0)
#endif
#ifndef DHPCD_GATEWAY
#define DHPCD_GATEWAY           DHCPD_OCTETS_TO_IP_ADDR(10,0,0,1)
#endif
#ifndef DHPCD_PRIMARY_DNS
#define DHPCD_PRIMARY_DNS       DHCPD_OCTETS_TO_IP_ADDR(10,0,0,5)
#endif
#ifndef DHPCD_SECONDARY_DNS
#define DHPCD_SECONDARY_DNS     DHCPD_OCTETS_TO_IP_ADDR(10,0,0,6)
#endif


/**
 * How long in seconds IP leases are for.
 */
#ifndef DHCPD_LEASE_TIME
#define DHCPD_LEASE_TIME (10 * 60)
#endif

/**
 * The amount of time in seconds an IP address is
 * reserved for after sending an offer to a client.
 * If a request is not received within this
 * amount of time the IP will be made available
 * again to new clients.
 */
#ifndef DHCPD_OFFER_EXPIRATION_TIME
#define DHCPD_OFFER_EXPIRATION_TIME 10
#endif

/**
 * How often in seconds to wake up and check to see if
 * leases have expired and if unavailable IP addresses
 * should be probed.
 */
#ifndef DHCPD_REFRESH_INTERVAL
#define DHCPD_REFRESH_INTERVAL 60
#endif

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
#ifndef DHCPD_PROBE_NEW_IP_ADDRESSES
#define DHCPD_PROBE_NEW_IP_ADDRESSES 1
#endif

/**
 * How often in seconds to probe IP addresses that
 * are marked as unavailable to determine if they
 * are still unavailable.
 *
 * Set to 0 to disable periodic probing.
 */
#ifndef DHCPD_UNAVAILABLE_IP_PROBE_INTERVAL
#define DHCPD_UNAVAILABLE_IP_PROBE_INTERVAL (5 * 60)
#endif

/**
 * The number of probes to send when checking to
 * see if an IP is in use.
 */
#ifndef DHCPD_IP_PROBE_ATTEMPTS
#define DHCPD_IP_PROBE_ATTEMPTS 2
#endif

/**
 * How long in milliseconds to wait between probes
 * when checking to see if an IP is in use.
 */
#ifndef DHCPD_IP_PROBE_WAIT_TIME
#define DHCPD_IP_PROBE_WAIT_TIME 250
#endif

/**
 * If either DHCPD_PROBE_NEW_IP_ADDRESSES or DHCPD_UNAVAILABLE_IP_PROBE_INTERVAL
 * are not set to 0, then this function must be called when a ping reply is
 * received.
 *
 * \param ping_number_in The ping number the reply is in response to.
 */
void dhcpd_ping_reply_received(uint16_t ping_number_in);

/**
 * Starts the DHCP server.
 *
 * \param priority The task priority to use for the server
 */
void dhcpd_start(unsigned priority);

/**
 * Stops the DHCP server.
 */
void dhcpd_stop(void);


#endif /* DHCPD_H_ */
