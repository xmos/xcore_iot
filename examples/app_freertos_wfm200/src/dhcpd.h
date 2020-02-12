/*
 * dhcpd.h
 *
 *  Created on: Feb 6, 2020
 *      Author: mbruno
 */


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

#define DHCPD_LEASE_TIME 120 // 64800 /* 18 hours */

#define DHCPD_CLIENT_COUNT_MAX 10

void dhcpd_start(UBaseType_t priority);
void dhcpd_stop();

//////////////////////////////////////
void dhcpd_client_list_test(void);

#endif /* DHCPD_H_ */
