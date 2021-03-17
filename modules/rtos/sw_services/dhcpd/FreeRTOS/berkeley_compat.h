// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef BERKELEY_COMPAT_H_
#define BERKELEY_COMPAT_H_

#define ntohl(x) FreeRTOS_ntohl(x)
#define htonl(x) FreeRTOS_htonl(x)
#define ntohs(x) FreeRTOS_ntohs(x)
#define htons(x) FreeRTOS_htons(x)

/* Internet address. */
struct in_addr {
    uint32_t s_addr;     /* address in network byte order */
};

static inline char *inet_ntoa(struct in_addr ina)
{
    static char a[32];
    FreeRTOS_inet_ntoa(ina.s_addr, a);
    return a;
}

inline int inet_aton(const char *cp, struct in_addr *inp)
{
    inp->s_addr = FreeRTOS_inet_addr(cp);
    if (inp->s_addr == 0) {
        return 0;
    }

    return 1;
}

struct sockaddr_in
{
    uint8_t sin_len;        /* length of this structure. */
    uint8_t sin_family;     /* FREERTOS_AF_INET. */
    uint16_t sin_port;
    struct in_addr sin_addr;
};

#define AF_INET FREERTOS_AF_INET
#define SOCK_DGRAM FREERTOS_SOCK_DGRAM
#define SOCK_STREAM FREERTOS_SOCK_STREAM

#define sockaddr freertos_sockaddr

#define INADDR_BROADCAST ((uint32_t)0xffffffffUL)
#define INADDR_ANY       ((uint32_t)0x00000000UL)

inline int socket(int domain, int type, int protocol)
{
    Socket_t s;

    if (type == FREERTOS_SOCK_STREAM) {
        protocol = FREERTOS_IPPROTO_TCP;
    } else if (type == FREERTOS_SOCK_DGRAM) {
        protocol = FREERTOS_IPPROTO_UDP;
    }

    s = FreeRTOS_socket(domain, type, protocol);
    return (int) s;
}

#define setsockopt(sockfd, level, optname, optval, optlen) FreeRTOS_setsockopt((Socket_t) (sockfd), level, optname, optval, optlen)
#define sendto(sockfd, buf, len, flags, dest_addr, addrlen) FreeRTOS_sendto((Socket_t) (sockfd), buf, len, flags, dest_addr, addrlen)
#define recvfrom(sockfd, buf, len, flags, src_addr, addrlen) FreeRTOS_recvfrom((Socket_t) (sockfd), buf, len, flags, src_addr, addrlen)
#define bind(sockfd, addr, addrlen) FreeRTOS_bind((Socket_t) (sockfd), addr, addrlen)
#define close(sockfd) FreeRTOS_closesocket((Socket_t) (sockfd))

#endif /* BERKELEY_COMPAT_H_ */
