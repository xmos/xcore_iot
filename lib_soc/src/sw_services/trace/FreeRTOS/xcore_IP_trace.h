/*
 * xcore_IP_trace.h
 *
 *  Created on: Oct 23, 2019
 *      Author: jmccarthy
 */


#ifndef XCORE_IP_TRACE_H_
#define XCORE_IP_TRACE_H_

#define iptraceNETWORK_DOWN()                                               tracePRINTF_LOG("IP: Network Down");
#define iptraceNETWORK_BUFFER_RELEASED( pxBufferAddress )                   tracePRINTF_LOG("IP: Network buffer release at %p", pxBufferAddress);
#define iptraceNETWORK_BUFFER_OBTAINED( pxBufferAddress )                   tracePRINTF_LOG("IP: Network buffer obtained at %p", pxBufferAddress);
#define iptraceNETWORK_BUFFER_OBTAINED_FROM_ISR( pxBufferAddress )          tracePRINTF_LOG("IP: Network buffer obtained from ISR at %p", pxBufferAddress);
#define iptraceFAILED_TO_OBTAIN_NETWORK_BUFFER()                            tracePRINTF_LOG("IP: Network failed to be obtained");
#define iptraceFAILED_TO_OBTAIN_NETWORK_BUFFER_FROM_ISR()                   tracePRINTF_LOG("IP: Network failed to be obtained by ISR");
#define iptraceCREATING_ARP_REQUEST( ulIPAddress )                          tracePRINTF_LOG("IP: Creating ARP request for %u", ulIPAddress);
#define iptraceARP_TABLE_ENTRY_WILL_EXPIRE( ulIPAddress )                   tracePRINTF_LOG("IP: ARP entry for %u will expire", ulIPAddress);
#define iptraceARP_TABLE_ENTRY_EXPIRED( ulIPAddress )                       tracePRINTF_LOG("IP: ARP entry for %u has expired", ulIPAddress);
#define iptraceARP_TABLE_ENTRY_CREATED( ulIPAddress, ucMACAddress )         tracePRINTF_LOG("IP: ARP entry created for %u ", ulIPAddress);
#define iptraceSENDING_UDP_PACKET( ulIPAddress )                            tracePRINTF_LOG("IP: Sending UDP packet to %u", ulIPAddress);
#define iptracePACKET_DROPPED_TO_GENERATE_ARP( ulIPAddress )                tracePRINTF_LOG("IP: Dropped packet to generate ARP for %u", ulIPAddress);
#define iptraceICMP_PACKET_RECEIVED()                                       tracePRINTF_LOG("IP: ICMP received");
#define iptraceSENDING_PING_REPLY( ulIPAddress )                            tracePRINTF_LOG("IP: Sending ping reply to %u", ulIPAddress);
#define traceARP_PACKET_RECEIVED()                                          tracePRINTF_LOG("IP: ARP received");
#define iptracePROCESSING_RECEIVED_ARP_REPLY( ulIPAddress )                 tracePRINTF_LOG("IP: Received ARP reply from %u", ulIPAddress);
#define iptraceSENDING_ARP_REPLY( ulIPAddress )                             tracePRINTF_LOG("IP: Sending ARP reply to %u", ulIPAddress);
#define iptraceFAILED_TO_CREATE_SOCKET()                                    tracePRINTF_LOG("IP: Failed to create socket");
#define iptraceFAILED_TO_CREATE_EVENT_GROUP()                               tracePRINTF_LOG("IP: Failed to create event group");
#define iptraceRECVFROM_DISCARDING_BYTES( xNumberOfBytesDiscarded )         tracePRINTF_LOG("IP: recvfrom discarded %d bytes", xNumberOfBytesDiscarded);
#define iptraceETHERNET_RX_EVENT_LOST()                                     tracePRINTF_LOG("IP: Ethernet rx event lost");
#define iptraceSTACK_TX_EVENT_LOST( xEvent )                                tracePRINTF_LOG("IP: stack tx event lost");
#define iptraceNETWORK_EVENT_RECEIVED( eEvent )                             tracePRINTF_LOG("IP: Network event received");
#define iptraceBIND_FAILED( xSocket, usPort )                               tracePRINTF_LOG("IP: Bind socket %p to port %u failed", xSocket, usPort);
#define iptraceDHCP_REQUESTS_FAILED_USING_DEFAULT_IP_ADDRESS( ulIPAddress ) tracePRINTF_LOG("IP: DHCP failed using %u", ulIPAddress);
#define iptraceSENDING_DHCP_DISCOVER()                                      tracePRINTF_LOG("IP: DHCP discover");
#define iptraceSENDING_DHCP_REQUEST()                                       tracePRINTF_LOG("IP: DHCP request");
#define iptraceDHCP_SUCCEDEED( address )                                    tracePRINTF_LOG("IP: DHCP succeeded");
#define iptraceNETWORK_INTERFACE_TRANSMIT()                                 tracePRINTF_LOG("IP: Network interface tx");
#define iptraceNETWORK_INTERFACE_RECEIVE()                                  tracePRINTF_LOG("IP: Network interface rx");
#define iptraceSENDING_DNS_REQUEST()                                        tracePRINTF_LOG("IP: Sending DNS request");
#define iptraceWAITING_FOR_TX_DMA_DESCRIPTOR()                              tracePRINTF_LOG("IP: Waiting for tx dma descriptor");
#define iptraceFAILED_TO_NOTIFY_SELECT_GROUP( xSocket )                     tracePRINTF_LOG("IP: Failed to notify select group for socket %p", xSocket);
#define iptraceRECVFROM_TIMEOUT()                                           tracePRINTF_LOG("IP: recvfrom timeout");
#define iptraceRECVFROM_INTERRUPTED()                                       tracePRINTF_LOG("IP: recvfrom interrupted");
#define iptraceNO_BUFFER_FOR_SENDTO()                                       tracePRINTF_LOG("IP: no buffer for sendto");
#define iptraceSENDTO_SOCKET_NOT_BOUND()                                    tracePRINTF_LOG("IP: Send to unbound socket");
#define iptraceSENDTO_DATA_TOO_LONG()                                       tracePRINTF_LOG("IP: Data too long");

#endif /* XCORE_IP_TRACE_H_ */
