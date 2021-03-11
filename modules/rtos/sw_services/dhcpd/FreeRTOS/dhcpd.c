// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT DHCPD

#include "FreeRTOS.h"
#include "list.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_ARP.h"

#include "berkeley_compat.h"

#include "dhcpd.h"

#include <string.h>
#include <stdint.h>

#define HWADDR_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define HWADDR_ARG(hwaddr) hwaddr.ucBytes[0], hwaddr.ucBytes[1], hwaddr.ucBytes[2], hwaddr.ucBytes[3], hwaddr.ucBytes[4], hwaddr.ucBytes[5]

#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68

#define DHCP_OPTIONS_LENGTH 308

/**
 * DHCP message as defined by RFC 2131.
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     op (1)    |   htype (1)   |   hlen (1)    |   hops (1)    |
 *  +---------------+---------------+---------------+---------------+
 *  |                            xid (4)                            |
 *  +-------------------------------+-------------------------------+
 *  |           secs (2)            |           flags (2)           |
 *  +-------------------------------+-------------------------------+
 *  |                          ciaddr  (4)                          |
 *  +---------------------------------------------------------------+
 *  |                          yiaddr  (4)                          |
 *  +---------------------------------------------------------------+
 *  |                          siaddr  (4)                          |
 *  +---------------------------------------------------------------+
 *  |                          giaddr  (4)                          |
 *  +---------------------------------------------------------------+
 *  |                                                               |
 *  |                          chaddr  (16)                         |
 *  |                                                               |
 *  |                                                               |
 *  +---------------------------------------------------------------+
 *  |                                                               |
 *  |                          sname   (64)                         |
 *  +---------------------------------------------------------------+
 *  |                                                               |
 *  |                          file    (128)                        |
 *  +---------------------------------------------------------------+
 *  |                                                               |
 *  |                          options (variable)                   |
 *  +---------------------------------------------------------------+
 */
typedef struct {
    uint8_t        op;           /* Message op code / message type.
                                    1 = BOOTREQUEST, 2 = BOOTREPLY */
    uint8_t        htype;        /* Hardware address type, see ARP section in "Assigned
                                    Numbers" RFC; e.g., '1' = 10mb ethernet. */
    uint8_t        hlen;         /* Hardware address length (e.g.  '6' for 10mb
                                    ethernet). */
    uint8_t        hops;         /* Client sets to zero, optionally used by relay agents
                                    when booting via a relay agent. */
    uint32_t       xid;          /* Transaction ID, a random number chosen by the
                                    client, used by the client and server to associate
                                    messages and responses between a client and a
                                    server. */
    uint16_t       secs;         /* Filled in by client, seconds elapsed since client
                                    began address acquisition or renewal process. */
    uint16_t       flags;        /* Flags (see figure 2). */
    struct in_addr ciaddr;       /* Client IP address; only filled in if client is in
                                    BOUND, RENEW or REBINDING state and can respond
                                    to ARP requests. */
    struct in_addr yiaddr;       /* 'your' (client) IP address. */
    struct in_addr siaddr;       /* IP address of next server to use in bootstrap;
                                    returned in DHCPOFFER, DHCPACK by server. */
    struct in_addr giaddr;       /* Relay agent IP address, used in booting via a
                                    relay agent. */
    MACAddress_t   chaddr;       /* Client hardware address. */
    uint8_t        padding[10];  /* Client hardware address padding. Technically part
                                    of the chaddr field. */
    uint8_t        sname[64];    /* Optional server host name, null terminated string. */
    uint8_t        file[128];    /* Boot file name, null terminated string; "generic"
                                    name or null in DHCPDISCOVER, fully qualified
                                    directory-path name in DHCPOFFER. */
    uint32_t       magic_cookie; /* Magic Cookie (99.130.83.99). Technically part of
                                    options field. */
    uint8_t        options[DHCP_OPTIONS_LENGTH]; /* Optional parameters field.  See the options
                                                    documents for a list of defined options. */
} dhcp_message_t;

/**
 * The minimum length for a DHCP message is one that contains
 * a single DHCP Message Type option followed by an END option.
 */
#define DHCP_REQUEST_MIN_LENGTH (sizeof(dhcp_message_t) - DHCP_OPTIONS_LENGTH + 3 + 1)

#define BOOTREQUEST      1
#define BOOTREPLY        2

#define DHCP_DISCOVER    1
#define DHCP_OFFER       2
#define DHCP_REQUEST     3
#define DHCP_DECLINE     4
#define DHCP_ACK         5
#define DHCP_NAK         6
#define DHCP_RELEASE     7
#define DHCP_INFORM      8

#define DHCP_MAGIC_COOKIE                 FreeRTOS_inet_addr_quick(99,130,83,99)
#define DHCP_OPTION_PAD                     0
#define DHCP_OPTION_SUBNET_MASK             1
#define DHCP_OPTION_ROUTER                  3
#define DHCP_OPTION_DOMAIN_NAME_SERVER      6
#define DHCP_OPTION_INTERFACE_MTU          26
#define DHCP_OPTION_BROADCAST_ADDRESS      28
#define DHCP_OPTION_REQUESTED_IP_ADDRESS   50
#define DHCP_OPTION_IP_ADDRESS_LEASE_TIME  51
#define DHCP_OPTION_DHCP_MESSAGE_TYPE      53
#define DHCP_OPTION_SERVER_IDENTIFIER      54
#define DHCP_OPTION_PARAMETER_REQ_LIST     55
#define DHCP_OPTION_MESSAGE                56
#define DHCP_OPTION_RENEWAL_TIME_VALUE     58
#define DHCP_OPTION_REBINDING_TIME_VALUE   59
#define DHCP_OPTION_END                   255

#define DHCP_IP_STATE_AVAILABLE      0
#define DHCP_IP_STATE_LEASED         1
#define DHCP_IP_STATE_OFFERED        2
#define DHCP_IP_STATE_STATIC         3
#define DHCP_IP_STATE_UNAVAILABLE    4

static QueueHandle_t ping_reply_queue;
static SemaphoreHandle_t dhcpd_lock;
static int dhcpd_socket = -1;

static struct in_addr dhcpd_server_ip;
static struct in_addr dhcpd_netmask;
static struct in_addr dhcpd_gateway;
static struct in_addr dhcpd_dns[2];
static struct in_addr dhcpd_ip_pool_start;
static struct in_addr dhcpd_ip_pool_end;

typedef struct {
    struct in_addr ip;
    int state;
    MACAddress_t mac;
    ListItem_t list_item;
} dhcp_ip_info_t;

static List_t dhcp_ip_pool;
static dhcp_ip_info_t *dhcp_ip_infos;


static int dhcpd_lock_get(void)
{
    int ret;

    if (dhcpd_lock != NULL) {
        if (xSemaphoreTakeRecursive(dhcpd_lock, portMAX_DELAY) == pdPASS) {
            ret = 0;
        } else {
            ret = -1;
        }
    } else {
        ret = -1;
    }

    return ret;
}

static void dhcpd_lock_release(void)
{
    if (dhcpd_lock != NULL) {
        xSemaphoreGiveRecursive(dhcpd_lock);
    }
}

static void dhcp_client_update(dhcp_ip_info_t *client, int state, uint32_t expiration)
{
    rtos_time_t now = rtos_time_get();

    uxListRemove(&client->list_item);
    listSET_LIST_ITEM_VALUE(&client->list_item, now.seconds + expiration);
    client->state = state;
    vListInsert(&dhcp_ip_pool, &client->list_item);
}

static dhcp_ip_info_t *dhcp_client_lookup_by_mac(const MACAddress_t *mac)
{
    int i;
    dhcp_ip_info_t *client;
    ListItem_t *item;

    item = listGET_HEAD_ENTRY(&dhcp_ip_pool);

    for (i = 0; i < listCURRENT_LIST_LENGTH(&dhcp_ip_pool); i++) {
        client = listGET_LIST_ITEM_OWNER(item);
        if (memcmp(&client->mac, mac, sizeof(client->mac)) == 0) {
            return client;
        }
        item = listGET_NEXT(item);
    }

    return NULL;
}

static dhcp_ip_info_t *dhcp_client_lookup_by_ip(struct in_addr ip)
{
    int i;
    dhcp_ip_info_t *client;
    ListItem_t *item;

    item = listGET_HEAD_ENTRY(&dhcp_ip_pool);

    for (i = 0; i < listCURRENT_LIST_LENGTH(&dhcp_ip_pool); i++) {
        client = listGET_LIST_ITEM_OWNER(item);
        if (client->ip.s_addr == ip.s_addr) {
            return client;
        }
        item = listGET_NEXT(item);
    }

    return NULL;
}

static dhcp_ip_info_t *dhcp_client_get_oldest_disconnected(void)
{
    int i;
    dhcp_ip_info_t *client;
    ListItem_t *item;

    item = listGET_HEAD_ENTRY(&dhcp_ip_pool);

    for (i = 0; i < listCURRENT_LIST_LENGTH(&dhcp_ip_pool); i++) {
        client = listGET_LIST_ITEM_OWNER(item);
        if (client->state == DHCP_IP_STATE_AVAILABLE) {
            return client;
        }
        item = listGET_NEXT(item);
    }

    return NULL;
}

static volatile uint16_t ping_number_out;

void dhcpd_ping_reply_received(uint16_t ping_number_in)
{
    if (ping_reply_queue != NULL && ping_number_out != pdFAIL && ping_number_in == ping_number_out) {
        xQueueOverwrite(ping_reply_queue, &ping_number_in);
    }
}

#if DHCPD_PROBE_NEW_IP_ADDRESSES || DHCPD_UNAVAILABLE_IP_PROBE_INTERVAL
static int dhcpd_ip_address_in_use(struct in_addr ip)
{
    int in_use = 0;
    int i;
    const int tries = 2;

    uint32_t arp_ip = ip.s_addr;
    MACAddress_t mac;

    if (eARPGetCacheEntry(&arp_ip, &mac) != eARPCacheHit) {
        /*
         * This IP does not have an entry in the ARP table
         * so probe it using ARP requests. If an entry appears
         * then the IP is in use.
         */
        const int n = DHCPD_IP_PROBE_WAIT_TIME / 10;

        for (i = 0; i < tries; i++) {
            int j;
            TickType_t last_wake;

            rtos_printf("\t%s unknown, will probe using ARP\n", inet_ntoa(ip));
            FreeRTOS_OutputARPRequest(arp_ip);

            last_wake = xTaskGetTickCount();
            for (j = 0; j < n; j++) {
                vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(10));
                if (eARPGetCacheEntry(&arp_ip, &mac) == eARPCacheHit) {
                    in_use = 1;
                    break;
                }
            }
            if (in_use) {
                break;
            }
        }
    } else {
        /*
         * This IP has an entry in the ARP table. This suggests
         * that the IP is in use. Check to see if it is still
         * around by sending it ICMP requests.
         */
        BaseType_t ret = pdFAIL;
        uint16_t ping_number_in;

        for (i = 0; i < tries; i++) {
            rtos_printf("\t%s is at " HWADDR_FMT ", will probe using ICMP\n", inet_ntoa(ip), HWADDR_ARG(mac));
            ping_number_out = FreeRTOS_SendPingRequest(ip.s_addr, 48, pdMS_TO_TICKS(100));
            if (ping_number_out != pdFAIL) {
                /* This loop will skip replies to previous pings */
                do {
                    ret = xQueueReceive(ping_reply_queue, &ping_number_in, pdMS_TO_TICKS(DHCPD_IP_PROBE_WAIT_TIME));
                } while (ret == pdPASS && ping_number_out != ping_number_in);
            }

            if (ret == pdPASS) {
                in_use = 1;
                break;
            }
        }

        ping_number_out = pdFAIL;
    }

    if (in_use) {
        rtos_printf("\tIP %s found on the network\n", inet_ntoa(ip));
    }

    return in_use;
}
#endif

static void dhcp_client_release_expired_leases(void)
{
    int i;
    dhcp_ip_info_t *client;
    ListItem_t *item;
    time_t expiration;
    rtos_time_t now = rtos_time_get();

    item = listGET_HEAD_ENTRY(&dhcp_ip_pool);

    for (i = 0; i < listCURRENT_LIST_LENGTH(&dhcp_ip_pool); i++) {
        client = listGET_LIST_ITEM_OWNER(item);
        expiration = listGET_LIST_ITEM_VALUE(item);
        if (now.seconds >= expiration) {
            if (client->state == DHCP_IP_STATE_LEASED || client->state == DHCP_IP_STATE_OFFERED) {
                client->state = DHCP_IP_STATE_AVAILABLE;
                rtos_printf("\tLease of %s to " HWADDR_FMT " has expired\n", inet_ntoa(client->ip), HWADDR_ARG(client->mac));
            }
#if DHCPD_UNAVAILABLE_IP_PROBE_INTERVAL
            else if (client->state == DHCP_IP_STATE_UNAVAILABLE) {
                if (dhcpd_ip_address_in_use(client->ip)) {
                    rtos_printf("\t%s is still in use.\n", inet_ntoa(client->ip));
                    dhcp_client_update(client, DHCP_IP_STATE_UNAVAILABLE, DHCPD_UNAVAILABLE_IP_PROBE_INTERVAL);
                } else {
                    rtos_printf("\t%s is no longer in use, setting as available.\n", inet_ntoa(client->ip));
                    client->state = DHCP_IP_STATE_AVAILABLE;
                }
            }
#endif
        }
        item = listGET_NEXT(item);
    }
}

static int netmask_valid(struct in_addr netmask)
{
    uint32_t mask;
    uint32_t nm_hostorder = htonl(netmask.s_addr);

    /* first, check for the first zero */
    for (mask = 1UL << 31 ; mask != 0; mask >>= 1) {
        if ((nm_hostorder & mask) == 0) {
            break;
        }
    }

    /* then check that there is no one */
    for (; mask != 0; mask >>= 1) {
        if ((nm_hostorder & mask) != 0) {
            /* there is a one after the first zero -> invalid */
            return 0;
        }
    }

    /* no one after the first zero -> valid */
    return 1;
}

inline int ip_netmask_compare(struct in_addr addr1, struct in_addr addr2, struct in_addr mask)
{
    return (addr1.s_addr & mask.s_addr) == (addr2.s_addr & mask.s_addr);
}

/**
 * This function handles selecting an IP address for a client
 * given its MAC address, the IP address the client has requested,
 * and the DHCP message type received from the client, which may
 * be one of DISCOVER, REQUEST, or INFORM.
 *
 * The selected IP is returned, and the IP pool is updated
 * with the client information (MAC address, current state,
 * lease expiration time). If there are no IP addresses
 * available then an all zero IP is returned. If the client
 * has sent a REQUEST message and is requesting an IP address
 * that was not offered, then the broadcast address is returned
 * which means a NAK should be sent.
 *
 * Whenever an IP is leased out, either for the first time or
 * for a renewal, the ARP cache is updated.
 *
 * If DHCPD_PROBE_NEW_IP_ADDRESSES is true then this function will
 * verify that the IP address it hands out is not already in use
 * on the network by sending out either an ICMP echo request or
 * ARP request (see dhcpd_ip_address_in_use()).
 *
 * \note This function has become a bit messy and difficult to
 * follow. It might be worth restructuring.
 */
static struct in_addr dhcpd_client_ip_address_lease(const MACAddress_t *mac, struct in_addr requested_ip, int request_type)
{
    dhcp_ip_info_t *dhcp_client;
    const struct in_addr zero_ip = {INADDR_ANY};
    const struct in_addr bad_ip = {INADDR_BROADCAST};

    dhcp_client = dhcp_client_lookup_by_mac(mac);
    if (dhcp_client != NULL) {
        /* Client was previously assigned an address and is still in the list.
        Return the address it was previously assigned. */

        if (request_type == DHCP_REQUEST && dhcp_client->ip.s_addr != requested_ip.s_addr) {
            /* If the requested IP address in a REQUEST message does not match
            the IP that was already either offered or leased to the client,
            then return the "bad" IP so that a NAK will be sent. */
            rtos_printf("\tClient requesting an IP address it has not been offered\n");
            return bad_ip;
        } else if (request_type != DHCP_INFORM) {
            uint32_t expiration = request_type == DHCP_DISCOVER ? DHCPD_OFFER_EXPIRATION_TIME : DHCPD_LEASE_TIME;
            int state = request_type == DHCP_DISCOVER ? DHCP_IP_STATE_OFFERED : DHCP_IP_STATE_LEASED;

#if DHCPD_PROBE_NEW_IP_ADDRESSES
            if (request_type == DHCP_DISCOVER && dhcp_client->state != DHCP_IP_STATE_LEASED && dhcpd_ip_address_in_use(dhcp_client->ip)) {
                /* The IP is in use on the network, even though it is not leased,
                and not in use by the client since it has sent a discover message. */
                memset(&dhcp_client->mac, 0, sizeof(dhcp_client->mac));
                dhcp_client_update(dhcp_client, DHCP_IP_STATE_UNAVAILABLE, DHCPD_UNAVAILABLE_IP_PROBE_INTERVAL);
                if (requested_ip.s_addr == dhcp_client->ip.s_addr) {
                    /* Since the IP we had associated with this client's MAC address is
                    in use elsewhere on the network, if the client is also requesting
                    this IP then do not attempt to honor the request below. */
                    requested_ip.s_addr = zero_ip.s_addr;
                }
            } else
#endif
            {
                dhcp_client_update(dhcp_client, state, expiration);
                rtos_printf("\tClient found. %s IP %s\n", state == DHCP_IP_STATE_OFFERED ? "Offering" : "Leasing", inet_ntoa(dhcp_client->ip));
                if (state == DHCP_IP_STATE_LEASED) {
                    rtos_printf("\tUpdating ARP cache entry (" HWADDR_FMT ")\n", HWADDR_ARG(dhcp_client->mac));
                    vARPRefreshCacheEntry(&dhcp_client->mac, dhcp_client->ip.s_addr);
                }
                return dhcp_client->ip;
            }
        } else {
            /* This is an inform message */
            if (dhcp_client->ip.s_addr == requested_ip.s_addr) {
                /* The client is telling us that its IP address matches what we already knew.
                Ensure its IP is set to static rather than leased. */
                dhcp_client_update(dhcp_client, DHCP_IP_STATE_STATIC, 0);
                rtos_printf("\tClient informing us it is using already assigned IP %s.\n", inet_ntoa(dhcp_client->ip));
                return dhcp_client->ip;
            } else {
                /* The client is telling us that its IP address is something other than what
                we thought we knew. Ensure the client's MAC address is disassociated with
                the old IP address. Below we will assign it to the IP address it is informing
                us with if it is available in the pool. */
                rtos_printf("\tClient informing us it is using a new IP.\n");
                memset(&dhcp_client->mac, 0, sizeof(dhcp_client->mac));
                dhcp_client_update(dhcp_client, DHCP_IP_STATE_AVAILABLE, 0);
            }
        }
    }

    /* An IP address has not yet been assigned this client's MAC address. */
    if (request_type == DHCP_DISCOVER || request_type == DHCP_INFORM) {
        if (requested_ip.s_addr != zero_ip.s_addr &&
                (dhcp_client = dhcp_client_lookup_by_ip(requested_ip)) != NULL &&
                dhcp_client->state == DHCP_IP_STATE_AVAILABLE) {
            /* The Client has requested a specific IP address,
            it is in the pool, and it is available. */

            if (request_type == DHCP_DISCOVER) {
                /* Verify that this IP is not already in use on the network */

#if DHCPD_PROBE_NEW_IP_ADDRESSES
                if (dhcpd_ip_address_in_use(dhcp_client->ip)) {
                    /* The IP is in use even though it is not leased */
                    memset(&dhcp_client->mac, 0, sizeof(dhcp_client->mac));
                    dhcp_client_update(dhcp_client, DHCP_IP_STATE_UNAVAILABLE, DHCPD_UNAVAILABLE_IP_PROBE_INTERVAL);
                } else
#endif
                {
                    dhcp_client->mac = *mac;
                    dhcp_client_update(dhcp_client, DHCP_IP_STATE_OFFERED, DHCPD_OFFER_EXPIRATION_TIME);
                    rtos_printf("\tClient not found. Offering requested IP %s\n", inet_ntoa(dhcp_client->ip));
                    return dhcp_client->ip;
                }
            } else {
                dhcp_client->mac = *mac;
                dhcp_client_update(dhcp_client, request_type == DHCP_IP_STATE_STATIC, 0);
                rtos_printf("\tClient informing us it is using available IP %s.\n", inet_ntoa(dhcp_client->ip));
                return dhcp_client->ip;
            }
        }

        if (request_type == DHCP_DISCOVER) {
            /* Either the client did not request a specific IP address,
            it wasn't in our pool, it has already been assigned or
            offered to another client, or the IP was found to be in use. */

            /* This forever loop looks dangerous but it is not possible
            for it to loop forever. Each time the next available IP address
            is determined to be in use it is marked as unavailable. Eventually
            either an available IP address will be found that is not in use, or
            all available IPs will be set as unavailable at which point
            dhcp_client_get_oldest_disconnected() WILL return NULL and zero_ip
            will be returned. */
            for (;;) {
                dhcp_client = dhcp_client_get_oldest_disconnected();
                if (dhcp_client != NULL) {
                    /* Verify that this IP is not already in use on the network */
#if DHCPD_PROBE_NEW_IP_ADDRESSES
                    if (dhcpd_ip_address_in_use(dhcp_client->ip)) {
                        /* The IP is in use even though it is not leased */
                        memset(&dhcp_client->mac, 0, sizeof(dhcp_client->mac));
                        dhcp_client_update(dhcp_client, DHCP_IP_STATE_UNAVAILABLE, DHCPD_UNAVAILABLE_IP_PROBE_INTERVAL);
                    } else
#endif
                    {
                        dhcp_client->mac = *mac;
                        dhcp_client_update(dhcp_client, DHCP_IP_STATE_OFFERED, DHCPD_OFFER_EXPIRATION_TIME);
                        rtos_printf("\tClient not found. Offering available IP %s\n", inet_ntoa(dhcp_client->ip));
                        return dhcp_client->ip;
                    }
                } else {
                    /* There are no IP addresses available for this client. Remain silent. */
                    rtos_printf("\tClient not found. There are no available IP addresses\n");
                    return zero_ip;
                }
            }
        } else {
            /* The IP address the client has informed us it is using is not available,
            so do not ACK, stay silent */
            rtos_printf("\tClient informed us it is using an unavailable IP address.\n");
            return zero_ip;
        }
    } else {
        /* If there is no record of a client that is sending
        a REQUEST message then the server should remain silent
        and not send either an ACK or NAK. */
        rtos_printf("\tUnknown client sent a REQUEST message\n");
        return zero_ip;
    }
}

/*
 * TODO:
   If the 'giaddr' field in a DHCP message from a client is non-zero,
   the server sends any return messages to the 'DHCP server' port on the
   BOOTP relay agent whose address appears in 'giaddr'. If the 'giaddr'
   field is zero and the 'ciaddr' field is nonzero, then the server
   unicasts DHCPOFFER and DHCPACK messages to the address in 'ciaddr'.
   If 'giaddr' is zero and 'ciaddr' is zero, and the broadcast bit is
   set, then the server broadcasts DHCPOFFER and DHCPACK messages to
   0xffffffff. If the broadcast bit is not set and 'giaddr' is zero and
   'ciaddr' is zero, then the server unicasts DHCPOFFER and DHCPACK
   messages to the client's hardware address and 'yiaddr' address.  In
   all cases, when 'giaddr' is zero, the server broadcasts any DHCPNAK
   messages to 0xffffffff.
 */
static void dhcpd_send_response(dhcp_message_t *dhcp_msg, size_t options_length)
{
    struct sockaddr_in dest_addr;
    int ret = 0;

    dest_addr.sin_port   = htons(DHCP_CLIENT_PORT);
    dest_addr.sin_family = AF_INET;

    if (dhcp_msg->ciaddr.s_addr != 0) {
        dest_addr.sin_addr = dhcp_msg->ciaddr;
    } else {
        dest_addr.sin_addr.s_addr = INADDR_BROADCAST;
    }

    ret = sendto(dhcpd_socket, dhcp_msg, sizeof(dhcp_message_t) - DHCP_OPTIONS_LENGTH + options_length, 0, (struct sockaddr *) &dest_addr, sizeof(dest_addr));

    /* check ret to print an error */
}

static void dhcp_option_add(dhcp_message_t *dhcp_msg,
                            uint8_t **opt_ptr,
                            uint8_t code,
                            size_t nmemb,
                            size_t size,
                            const void *data)
{
    const uint8_t *options_end;
    uint8_t *opt_tmp;
    const size_t option_len = nmemb * size;

    configASSERT(option_len <= 255);

    options_end = &dhcp_msg->options[sizeof(dhcp_msg->options) - 1];

    if (*opt_ptr == NULL) {
        opt_tmp = dhcp_msg->options;
    } else {
        opt_tmp = *opt_ptr;
    }

    configASSERT(opt_tmp <= options_end);
    *opt_tmp++ = code;

    if (option_len > 0) {
        configASSERT((opt_tmp + option_len) <= options_end);
        *opt_tmp++ = option_len;
        memcpy(opt_tmp, data, option_len);
        opt_tmp += option_len;
    }

    *opt_ptr = opt_tmp;
}

/*
 * TODO: Should look through the sname and/or file areas
 * for options when the Option Overload option is present.
 *
 * Honestly though, it is unnecessary.
 */
static int dhcp_option_get_next(const dhcp_message_t *dhcp_msg,
                                 const uint8_t **opt_ptr,
                                 size_t options_length,
                                 size_t *opt_len,
                                 const void **opt_data)
{
    int opt;
    const uint8_t *options_end;
    const uint8_t *opt_tmp;

    if (options_length > sizeof(dhcp_msg->options)) {
        options_length = sizeof(dhcp_msg->options);
    }
    options_end = &dhcp_msg->options[options_length - 1];

    if (*opt_ptr == NULL) {
        opt_tmp = dhcp_msg->options;
    } else {
        opt_tmp = *opt_ptr;
    }

    opt = *opt_tmp++;

    if (opt == DHCP_OPTION_END) {
        *opt_len = 0;
        *opt_data = NULL;
        return DHCP_OPTION_END;
    } else if (opt == DHCP_OPTION_PAD) {
        *opt_len = 0;
        *opt_data = NULL;

        if (opt_tmp <= options_end) {
            *opt_ptr = opt_tmp;
            return DHCP_OPTION_PAD;
        } else {
            rtos_printf("Last option is pad\n");
            return DHCP_OPTION_END;
        }
    } else {
        /*
         * Check that we do NOT read beyond the end of options.
         * If the length field or length of data field goes past
         * the end of the options array size then return 0.
         */
        if (opt_tmp > options_end) {
            rtos_printf("Missing length for opt %d\n", opt);
            *opt_len = 0;
            *opt_data = NULL;
            return DHCP_OPTION_END;
        }

        uint8_t len = *opt_tmp++;

        if (opt_tmp + len <= options_end) {
            *opt_len = len;
            *opt_data = opt_tmp;
            *opt_ptr = opt_tmp + len;
            return opt;
        } else {
            rtos_printf("Last option is %d\n", opt);
            *opt_len = 0;
            *opt_data = NULL;
            return DHCP_OPTION_END;
        }
    }
}

#define DHCP_CLIENT_STATE_NONE        0
#define DHCP_CLIENT_STATE_DISCOVER    1
#define DHCP_CLIENT_STATE_SELECTING   2
#define DHCP_CLIENT_STATE_INIT_REBOOT 3
#define DHCP_CLIENT_STATE_RENEWING    4
#define DHCP_CLIENT_STATE_REBINDING   5
#define DHCP_CLIENT_STATE_INFORMING   6
#define DHCP_CLIENT_STATE_RELEASING   7
#define DHCP_CLIENT_STATE_DECLINING   8
#define DHCP_CLIENT_STATE_PINGING     9

/*
 * This function is used to handle DHCP messages when the op
 * field is BOOTREQUEST.
 *
 * When the DHCP message type is one of:
 *  - DHCPDISCOVER
 *  - DHCPREQUEST
 *  - DHCPINFORM
 *
 *  It will respond with a DHCP message of one of the following types:
 *  - DHCPOFFER
 *  - DHCPACK
 *  - DHCPNAK
 *
 * When the DHCP message type is one of:
 *  - DHCPRELEASE
 *  - DHCPDECLINE
 *
 * It will not respond.
 */
static void dhcpd_handle_op_request(dhcp_message_t *dhcp_msg, size_t options_length)
{
    const uint8_t *opt_ptr = NULL;
    dhcp_ip_info_t *dhcp_client;
    int opt;
    int dhcp_msg_type = 0;
    int ip_requested = 0;
    int server_identified = 0;
    int state = DHCP_CLIENT_STATE_NONE;
    struct in_addr server_identifier = {INADDR_ANY};
    struct in_addr requested_ip = {INADDR_ANY};
    const uint8_t default_params[] = {
            DHCP_OPTION_SUBNET_MASK,
            DHCP_OPTION_ROUTER,
            DHCP_OPTION_DOMAIN_NAME_SERVER,
            DHCP_OPTION_INTERFACE_MTU,
            DHCP_OPTION_BROADCAST_ADDRESS,
            DHCP_OPTION_IP_ADDRESS_LEASE_TIME,
            DHCP_OPTION_RENEWAL_TIME_VALUE,
            DHCP_OPTION_REBINDING_TIME_VALUE,
    };
    const uint8_t *requested_params = default_params;
    size_t requested_params_count = sizeof(default_params);

    rtos_printf("DHCP message received\n");

    do {
        const void *opt_data;
        size_t opt_len;

        opt = dhcp_option_get_next(dhcp_msg, &opt_ptr, options_length, &opt_len, &opt_data);

        switch (opt) {
        case DHCP_OPTION_DHCP_MESSAGE_TYPE:
            if (opt_len == sizeof(uint8_t)) {
                dhcp_msg_type = *(uint8_t *)opt_data;
                rtos_printf("\tDHCP message type is %d\n", dhcp_msg_type);
            }
            break;
        case DHCP_OPTION_REQUESTED_IP_ADDRESS:
            if (opt_len == sizeof(struct in_addr)) {
                ip_requested = 1;
                memcpy(&requested_ip, opt_data, sizeof(struct in_addr));
                rtos_printf("\tRequesting IP address %s\n", inet_ntoa(requested_ip));
            }
            break;
        case DHCP_OPTION_SERVER_IDENTIFIER:
            if (opt_len == sizeof(struct in_addr)) {
                server_identified = 1;
                memcpy(&server_identifier, opt_data, sizeof(struct in_addr));
                rtos_printf("\tServer identifier IP address %s\n", inet_ntoa(server_identifier));
            }
            break;
        case DHCP_OPTION_PARAMETER_REQ_LIST:
            requested_params = opt_data;
            requested_params_count = opt_len;
            rtos_printf("\tOption parameter list provided\n");
            break;
        }
    } while (opt != DHCP_OPTION_END);

    switch (dhcp_msg_type) {
    case DHCP_DISCOVER:
        rtos_printf("\tClient is in the discover state.\n");
        state = DHCP_CLIENT_STATE_DISCOVER;
        break;

    case DHCP_REQUEST:
        if (ip_requested && server_identified && dhcp_msg->ciaddr.s_addr == 0) {
            if (server_identifier.s_addr == dhcpd_server_ip.s_addr) {
                rtos_printf("\tClient is in the selecting state.\n");
                state = DHCP_CLIENT_STATE_SELECTING;
            } else {
                rtos_printf("\tClient has selected a different server.\n");

                /* The client has chosen another server. Lookup its
                MAC address in the IP pool. It will be released. */
                dhcp_client = dhcp_client_lookup_by_mac(&dhcp_msg->chaddr);
                state = DHCP_CLIENT_STATE_RELEASING;
            }
        } else if (ip_requested && !server_identified && dhcp_msg->ciaddr.s_addr == 0) {
            rtos_printf("\tClient is in the init/reboot state.\n");
            state = DHCP_CLIENT_STATE_INIT_REBOOT;
        } else if (!ip_requested && !server_identified && dhcp_msg->ciaddr.s_addr != 0) {
            rtos_printf("\tClient is in the renewing/rebinding state.\n");
            requested_ip = dhcp_msg->ciaddr;

            /*
             * TODO: If this DHCP message was unicast, then the state is
             * renewing. If it was broadcast, it is rebinding. Currently
             * not able to check this, so assume it is renewing.
             */

            state = DHCP_CLIENT_STATE_RENEWING;
        } else if (ip_requested && !server_identified && dhcp_msg->ciaddr.s_addr != 0) {
            /* This is not a valid state according to RFC2131, but the dhcping program
            sends REQUEST messages like this. */
            rtos_printf("\tClient is in the ping state.\n");
            state = DHCP_CLIENT_STATE_PINGING;
        } else {
            rtos_printf("\tInvalid request message.\n\n");
        }
        break;

    case DHCP_INFORM:
        if (!ip_requested && !server_identified && dhcp_msg->ciaddr.s_addr != 0) {
            rtos_printf("\tClient is in the inform state.\n");
            requested_ip = dhcp_msg->ciaddr;
            state = DHCP_CLIENT_STATE_INFORMING;
        } else {
            rtos_printf("\tInvalid inform message.\n\n");
        }
        break;

    case DHCP_DECLINE:
        if (ip_requested && server_identified) {
            rtos_printf("\tClient is in the decline state.\n");
            /* Set the client's IP to zero to ensure its MAC and
            IP are disassociated. The client is supposed to set
            ciiaddr to zero in a decline message, but they don't
            always. */
            dhcp_msg->ciaddr.s_addr = 0;
            dhcp_client = dhcp_client_lookup_by_ip(requested_ip);
            state = DHCP_CLIENT_STATE_DECLINING;
        } else {
            rtos_printf("\tInvalid decline message.\n\n");
        }
        break;

    case DHCP_RELEASE:
        if (!ip_requested && server_identified && dhcp_msg->ciaddr.s_addr != 0) {
            rtos_printf("\tClient is in the release state.\n");
            dhcp_client = dhcp_client_lookup_by_ip(dhcp_msg->ciaddr);
            state = DHCP_CLIENT_STATE_RELEASING;
        } else {
            rtos_printf("\tInvalid release message.\n\n");
        }
        break;

    default:
        rtos_printf("\tUnrecognized DHCP message type %d\n\n", dhcp_msg_type);
        break;
    }

    switch (state) {
    case DHCP_CLIENT_STATE_DECLINING:
    case DHCP_CLIENT_STATE_RELEASING:
        if (dhcp_client != NULL) {
            if (dhcp_msg->ciaddr.s_addr == 0) {
                rtos_printf("\tDisassociating " HWADDR_FMT " from %s\n", HWADDR_ARG(dhcp_client->mac), inet_ntoa(dhcp_client->ip));
                memset(&dhcp_client->mac, 0, sizeof(dhcp_client->mac));
            }
            if (state == DHCP_CLIENT_STATE_DECLINING) {
                rtos_printf("\tMaking %s unavailable\n\n", inet_ntoa(dhcp_client->ip));
                dhcp_client_update(dhcp_client, DHCP_IP_STATE_UNAVAILABLE, DHCPD_UNAVAILABLE_IP_PROBE_INTERVAL);
            } else {
                rtos_printf("\tMaking %s available\n\n", inet_ntoa(dhcp_client->ip));
                dhcp_client_update(dhcp_client, DHCP_IP_STATE_AVAILABLE, 0);
            }
        } else {
            rtos_printf("\tIP to release/decline not found in pool\n\n");
        }
        /* Do not reply, return now */
        return;

    case DHCP_CLIENT_STATE_NONE:
        /* Either the DHCP message type was invalid or the
        combination of fields and message type was invalid.
        Just return now and remain silent. */
        return;

    case DHCP_CLIENT_STATE_PINGING:
        /* When the client is pinging, just set 'your' address to the
        requested address. Don't actually update the lease. */
        dhcp_msg->yiaddr = requested_ip;
        break;

    default:
        /* All other states should attempt to require a lease for the
        requested IP address. */
        dhcp_msg->yiaddr = dhcpd_client_ip_address_lease(&dhcp_msg->chaddr, requested_ip, dhcp_msg_type);
        break;
    }

    if (dhcp_msg->yiaddr.s_addr != INADDR_ANY && dhcp_msg->yiaddr.s_addr != INADDR_BROADCAST) {
        uint8_t *option_ptr = NULL;

        /* Indicates which options to include in the response */
        int include_subnet_mask = 0;
        int include_router = 0;
        int include_dns = 0;
        int include_mtu = 0;
        int include_broadcast = 0;
        int include_renewal_time = 0;
        int include_rebinding_time = 0;

        /* Option values */
        uint32_t lease_time = htonl(DHCPD_LEASE_TIME);
        uint32_t renewal_time = htonl(DHCPD_LEASE_TIME * 0.5);
        uint32_t rebinding_time = htonl(DHCPD_LEASE_TIME * 0.875);
        struct in_addr broadcast_ip = {INADDR_BROADCAST};
        uint16_t interface_mtu = htons(ipconfigNETWORK_MTU);

        /* Set these fields according to RFC2131 */
        dhcp_msg->op = BOOTREPLY;
        dhcp_msg->htype = 1; /* Ethernet */
        dhcp_msg->hlen = ipMAC_ADDRESS_LENGTH_BYTES;
        dhcp_msg->hops = 0;
        //dhcp_msg->xid = dhcp_msg->xid;
        dhcp_msg->secs = 0;
        if (state == DHCP_CLIENT_STATE_DISCOVER) {
            /* Ensure this is zero when sending an OFFER. */
            dhcp_msg->ciaddr.s_addr = 0;
        } else if (state == DHCP_CLIENT_STATE_INFORMING) {
            /* Ensure this is zero when replying to an INFORM. */
            dhcp_msg->yiaddr.s_addr = 0;
        }
        dhcp_msg->siaddr.s_addr = 0;
        //dhcp_msg->flags = dhcp_msg->flags;
        //dhcp_msg->giaddr = dhcp_msg->giaddr;
        //dhcp_msg->chaddr = dhcp_msg->chaddr;
        memset(dhcp_msg->sname, 0, sizeof(dhcp_msg->sname));
        memset(dhcp_msg->file, 0, sizeof(dhcp_msg->file));
        dhcp_msg->magic_cookie = DHCP_MAGIC_COOKIE;

        for (int i = 0; i < requested_params_count; i++) {
            switch (requested_params[i]) {
            case DHCP_OPTION_SUBNET_MASK:
                include_subnet_mask = 1;
                break;
            case DHCP_OPTION_ROUTER:
                include_router = 1;
                break;
            case DHCP_OPTION_DOMAIN_NAME_SERVER:
                include_dns = 1;
                break;
            case DHCP_OPTION_INTERFACE_MTU:
                include_mtu = 1;
                break;
            case DHCP_OPTION_BROADCAST_ADDRESS:
                include_broadcast = 1;
                break;
            case DHCP_OPTION_RENEWAL_TIME_VALUE:
                include_renewal_time = 1;
                break;
            case DHCP_OPTION_REBINDING_TIME_VALUE:
                include_rebinding_time = 1;
                break;
            }
        }

        /* These options MUST be present in the DHCPOFFER and DHCPACK messages according to RFC2131 */
        dhcp_msg_type = (state == DHCP_CLIENT_STATE_DISCOVER ? DHCP_OFFER : DHCP_ACK);
        dhcp_option_add(dhcp_msg, &option_ptr, DHCP_OPTION_DHCP_MESSAGE_TYPE, 1, sizeof(uint8_t),         &dhcp_msg_type);
        dhcp_option_add(dhcp_msg, &option_ptr, DHCP_OPTION_SERVER_IDENTIFIER, 1,  sizeof(struct in_addr), &dhcpd_server_ip);

        if (state != DHCP_CLIENT_STATE_INFORMING) {
            /* Except this option MUST NOT be present in the DHCPACK message when
            replying to a DHCPINFORM message */
            dhcp_option_add(dhcp_msg, &option_ptr, DHCP_OPTION_IP_ADDRESS_LEASE_TIME, 1, sizeof(uint32_t), &lease_time);
        }

        rtos_printf("\nSending a reply\n");
        rtos_printf("\tDHCP message type is %d\n", dhcp_msg_type);

        /*
         * If the client requested specific parameters, then only
         * set the options we support if they are in the client's
         * request list. If the client did not request a list then
         * a default list defined above is used.
         */
        if (include_subnet_mask) {
            rtos_printf("\tIncluding Subnet Mask\n");
            dhcp_option_add(dhcp_msg, &option_ptr, DHCP_OPTION_SUBNET_MASK, 1, sizeof(struct in_addr), &dhcpd_netmask);
        }
        if (include_router) {
            rtos_printf("\tIncluding Router\n");
            dhcp_option_add(dhcp_msg, &option_ptr, DHCP_OPTION_ROUTER, 1, sizeof(struct in_addr), &dhcpd_gateway);
        }
        if (include_dns) {
            rtos_printf("\tIncluding Domain Name Server\n");
            dhcp_option_add(dhcp_msg, &option_ptr, DHCP_OPTION_DOMAIN_NAME_SERVER, 1, sizeof(dhcpd_dns), dhcpd_dns);
        }
        if (include_mtu) {
            rtos_printf("\tIncluding MTU\n");
            dhcp_option_add(dhcp_msg, &option_ptr, DHCP_OPTION_INTERFACE_MTU, 1, sizeof(uint16_t), &interface_mtu);
        }
        if (include_broadcast) {
            rtos_printf("\tIncluding Broadcast Address\n");
            dhcp_option_add(dhcp_msg, &option_ptr, DHCP_OPTION_BROADCAST_ADDRESS, 1, sizeof(struct in_addr), &broadcast_ip);
        }
        if (include_renewal_time) {
            rtos_printf("\tIncluding Renewal Time\n");
            dhcp_option_add(dhcp_msg, &option_ptr, DHCP_OPTION_RENEWAL_TIME_VALUE, 1, sizeof(uint32_t), &renewal_time);
        }
        if (include_rebinding_time) {
            rtos_printf("\tIncluding Rebinding Time\n");
            dhcp_option_add(dhcp_msg, &option_ptr, DHCP_OPTION_REBINDING_TIME_VALUE, 1, sizeof(uint32_t), &rebinding_time);
        }

        /* The option list MUST end with the END option. */
        dhcp_option_add(dhcp_msg, &option_ptr, DHCP_OPTION_END, 0, 0, NULL);

        rtos_printf("\n");

        options_length = option_ptr - dhcp_msg->options;
        dhcpd_send_response(dhcp_msg, options_length);
    } else if (dhcp_msg->yiaddr.s_addr == INADDR_BROADCAST) {
        uint8_t *option_ptr = NULL;

        rtos_printf("\tRequested IP is invalid.\n");

        /* We will be sending a NAK because we already have the
        the client associated with an IP address, but it is not
        the one it requested. Ensure the IP we have associated
        with it is now set as available, but do not forget the
        association. If it sends another DISCOVER we will likely
        OFFER it. */

        dhcp_client = dhcp_client_lookup_by_mac(&dhcp_msg->chaddr);
        if (dhcp_client != NULL) {
            dhcp_client_update(dhcp_client, DHCP_IP_STATE_AVAILABLE, 0);
        }

        /* Set these fields according to RFC2131 */
        dhcp_msg->op = BOOTREPLY;
        dhcp_msg->htype = 1; /* Ethernet */
        dhcp_msg->hlen = ipMAC_ADDRESS_LENGTH_BYTES;
        dhcp_msg->hops = 0;
        //dhcp_msg->xid = dhcp_msg->xid;
        dhcp_msg->secs = 0;
        dhcp_msg->ciaddr.s_addr = 0;
        dhcp_msg->yiaddr.s_addr = 0;
        dhcp_msg->siaddr.s_addr = 0;
        //dhcp_msg->flags = dhcp_msg->flags;
        //dhcp_msg->giaddr = dhcp_msg->giaddr;
        //dhcp_msg->chaddr = dhcp_msg->chaddr;
        memset(dhcp_msg->sname, 0, sizeof(dhcp_msg->sname));
        memset(dhcp_msg->file, 0, sizeof(dhcp_msg->file));
        dhcp_msg->magic_cookie = DHCP_MAGIC_COOKIE;

        /* These options MUST be present in the DHCPNCK message according to RFC2131 */
        dhcp_msg_type = DHCP_NAK;
        dhcp_option_add(dhcp_msg, &option_ptr, DHCP_OPTION_DHCP_MESSAGE_TYPE, 1, sizeof(uint8_t),        &dhcp_msg_type);
        dhcp_option_add(dhcp_msg, &option_ptr, DHCP_OPTION_SERVER_IDENTIFIER, 1, sizeof(struct in_addr), &dhcpd_server_ip);
        dhcp_option_add(dhcp_msg, &option_ptr, DHCP_OPTION_END, 0, 0, NULL);

        rtos_printf("\nSending a reply\n");
        rtos_printf("\tDHCP message type is %d\n\n", dhcp_msg_type);

        options_length = option_ptr - dhcp_msg->options;
        dhcpd_send_response(dhcp_msg, options_length);
    } else {
        rtos_printf("\tClient is unrecognized. Remain silent.\n\n");
    }
}

static void dhcpd_listen(void)
{
    const TickType_t recv_timeout = pdMS_TO_TICKS(DHCPD_REFRESH_INTERVAL * 1000);
    TickType_t last_timeout;
    TickType_t now;

    last_timeout = xTaskGetTickCount();
    setsockopt(dhcpd_socket, 0, FREERTOS_SO_RCVTIMEO, &recv_timeout, sizeof(recv_timeout));

    while (dhcpd_socket != -1) {

        int ret;
        dhcp_message_t dhcp_msg;

        rtos_printf("DHCPD listening\n");

        ret = recvfrom(dhcpd_socket, &dhcp_msg, sizeof(dhcp_message_t), 0, NULL, NULL);

        dhcpd_lock_get();

        if (ret == -pdFREERTOS_ERRNO_EINTR) {
            /* dhcpd_stop() has been called */
            rtos_printf("Closing DHCPD socket\n");
            close(dhcpd_socket);
            vPortFree(dhcp_ip_infos);
            dhcpd_socket = -1;
        } else if (ret >= DHCP_REQUEST_MIN_LENGTH && dhcp_msg.op == BOOTREQUEST) {
            size_t options_length = ret - (sizeof(dhcp_msg) - sizeof(dhcp_msg.options));
            dhcpd_handle_op_request(&dhcp_msg, options_length);
        }

        now = xTaskGetTickCount();

        if (now >= (last_timeout + recv_timeout)) {
            last_timeout = now;
            rtos_printf("Checking for expired leases\n");
            dhcp_client_release_expired_leases();
        }

        dhcpd_lock_release();
    }
}

/*
 * Returns the next valid IP address from the pool
 * starting at ip.
 */
static struct in_addr dhcpd_ip_address_pool_next_valid(struct in_addr *pool)
{
    struct in_addr ip = *pool;
    uint32_t ipval;

    for (ipval = ntohl(ip.s_addr); ipval <= ntohl(dhcpd_ip_pool_end.s_addr); ipval++) {

        ip.s_addr = htonl(ipval);
        uint32_t octet4 = ipval & 0x000000FF;

        if (ipval < ntohl(dhcpd_ip_pool_start.s_addr)) {
            /* Bump the IP value up so that it will be at
            the start of the pool on the next iteration. */
            ipval = ntohl(dhcpd_ip_pool_start.s_addr) - 1;
        } else if (octet4 == 0 ||
                   octet4 == 0xFF ||
                   ip.s_addr == dhcpd_server_ip.s_addr ||
                   ip.s_addr == dhcpd_gateway.s_addr ||
                   ip.s_addr == dhcpd_dns[0].s_addr ||
                   ip.s_addr == dhcpd_dns[1].s_addr ) {

            /* This IP must be skipped. */

        } else {
            /* This IP is valid. */
            pool->s_addr = htonl(ipval + 1);
            break;
        }
    }

    if (ipval > ntohl(dhcpd_ip_pool_end.s_addr)) {
        /* If we are past the end of the pool then return
        the all zeros IP. */
        ip.s_addr = INADDR_ANY;
    }

    return ip;
}

/*
 * Creates the IP Pool
 */
static void dhcpd_ip_pool_init(void)
{
    struct in_addr pool = dhcpd_ip_pool_start;
    size_t ip_pool_count = ntohl(dhcpd_ip_pool_end.s_addr) - ntohl(dhcpd_ip_pool_start.s_addr);
    size_t ip_pool_size = sizeof(dhcp_ip_info_t) * ip_pool_count;

    dhcp_ip_infos = pvPortMalloc(ip_pool_size);
    memset(dhcp_ip_infos, 0, ip_pool_size);

    vListInitialise(&dhcp_ip_pool);

    for (int i = 0; i < ip_pool_count; i++) {
        struct in_addr ip;
        rtos_time_t now = rtos_time_get();

        ip = dhcpd_ip_address_pool_next_valid(&pool);
        if (ip.s_addr == INADDR_ANY) {
            break;
        }

        rtos_printf("Adding %s to IP pool\n", inet_ntoa(ip));

        dhcp_ip_infos[i].ip = ip;
        vListInitialiseItem(&dhcp_ip_infos[i].list_item);
        listSET_LIST_ITEM_OWNER(&dhcp_ip_infos[i].list_item, &dhcp_ip_infos[i]);
        listSET_LIST_ITEM_VALUE(&dhcp_ip_infos[i].list_item, now.seconds);

        vListInsert(&dhcp_ip_pool, &dhcp_ip_infos[i].list_item);
    }
}

static void dhcpd_task(void *param)
{
    (void) param;

    configASSERT(dhcpd_socket != -1);

    rtos_printf("DHCPD started\n");

    rtos_printf("Server IP %s\n", inet_ntoa(dhcpd_server_ip));
    rtos_printf("Netmask %s\n", inet_ntoa(dhcpd_netmask));
    rtos_printf("Gateway %s\n", inet_ntoa(dhcpd_gateway));
    rtos_printf("DNS1 %s\n", inet_ntoa(dhcpd_dns[0]));
    rtos_printf("DNS2 %s\n", inet_ntoa(dhcpd_dns[1]));
    rtos_printf("Start IP %s\n", inet_ntoa(dhcpd_ip_pool_start));
    rtos_printf("End IP %s\n", inet_ntoa(dhcpd_ip_pool_end));

    dhcpd_listen();

    vTaskDelete(NULL);
}

void dhcpd_start(unsigned priority)
{
    if (dhcpd_lock == NULL) {
        dhcpd_lock = xSemaphoreCreateRecursiveMutex();
        configASSERT(dhcpd_lock != NULL);
    }

    if (ping_reply_queue == NULL) {
        ping_reply_queue = xQueueCreate(1, sizeof(uint16_t));
        configASSERT(ping_reply_queue != NULL);
    }

    dhcpd_lock_get();

    if (dhcpd_socket == -1) {
        BaseType_t ret;
        struct sockaddr_in dhcpd_addr = {0};

        rtos_printf("dhcpd start\n");

        dhcpd_server_ip.s_addr = DHPCD_SERVER_IP;
        dhcpd_ip_pool_start.s_addr = DHPCD_IP_POOL_START;
        dhcpd_ip_pool_end.s_addr = DHPCD_IP_POOL_END;
        dhcpd_netmask.s_addr = DHPCD_NETMASK;
        dhcpd_gateway.s_addr = DHPCD_GATEWAY;
        dhcpd_dns[0].s_addr = DHPCD_PRIMARY_DNS;
        dhcpd_dns[1].s_addr = DHPCD_SECONDARY_DNS;

        configASSERT(netmask_valid(dhcpd_netmask));
        configASSERT(ip_netmask_compare(dhcpd_gateway, dhcpd_ip_pool_start, dhcpd_netmask));
        configASSERT(ip_netmask_compare(dhcpd_ip_pool_start, dhcpd_ip_pool_end, dhcpd_netmask));
        configASSERT(ntohl(dhcpd_ip_pool_start.s_addr) <= ntohl(dhcpd_ip_pool_end.s_addr));

        dhcpd_ip_pool_init();

        dhcpd_socket = socket(AF_INET, SOCK_DGRAM, 0);
        configASSERT(dhcpd_socket != -1);

        dhcpd_addr.sin_family = AF_INET;
        dhcpd_addr.sin_port = htons(DHCP_SERVER_PORT);
        dhcpd_addr.sin_addr.s_addr = INADDR_ANY;

        ret = bind(dhcpd_socket, (struct sockaddr *) &dhcpd_addr, sizeof(dhcpd_addr));
        configASSERT(ret == 0);

        xTaskCreate(dhcpd_task, DHCPD_TASK_NAME, RTOS_THREAD_STACK_SIZE(dhcpd_task), NULL, priority, NULL);
    } else {
        rtos_printf("dhcpd already started\n");
    }

    dhcpd_lock_release();
}

void dhcpd_stop(void)
{
    if (dhcpd_socket != -1) {
        rtos_printf("dhcpd stopping\n");

        /* Closing a socket that another task is waiting on
         * with FreeRTOS_recvfrom causes that task to get
         * stuck forever. It does not return with 0 or some
         * other error. Therefore send a signal to it here
         * and let it close the socket. Unfortunately this
         * isn't standard so will only work with FreeRTOS+TCP.
         */
        FreeRTOS_SignalSocket((Socket_t) dhcpd_socket);

        dhcpd_lock_get();
        while (dhcpd_socket != -1) {
            /* This will happen if the task that has called dhcpd_stop()
            has a higher priority than the DHCPD task. Pause for 1 tick
            here to give it a chance to run and close the socket. */
            rtos_printf("Waiting for DHCPD socket to close\n");
            dhcpd_lock_release();
            vTaskDelay(1);
            dhcpd_lock_get();
        }
        dhcpd_lock_release();
        rtos_printf("dhcpd stopped\n");
    } else {
        rtos_printf("dhcpd not started\n");
    }
}
