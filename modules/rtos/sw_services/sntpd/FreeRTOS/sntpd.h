// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef SRC_SNTPD_SNTPD_H_
#define SRC_SNTPD_SNTPD_H_

#define SNTPD_TASK_NAME "sntpd"
#define SNTPD_TASK_STACKSIZE ( portTASK_STACK_DEPTH( sntpd_task ) )

/* Set to standard NTP port if not defined elsewhere */
#ifndef SNTPD_PORT
#define SNTPD_PORT	123
#endif

/* Default SNTPD polling rate */
#ifndef SNTPD_POLL_RATE_MS
#define SNTPD_POLL_RATE_MS	15000
#endif

/* Default SNTPD timeout */
#ifndef SNTPD_RX_TIMEOUT_MS
#define SNTPD_RX_TIMEOUT_MS	5000
#endif

/* SNTPD will close the socket and try to find a valid time server after
 * x failed attempts */
#ifndef SNTPD_RESET_AFTER_X_FAILURES
#define SNTPD_RESET_AFTER_X_FAILURES 10
#endif

/* User may provide time servers list by defining SNTPD_USER_TIME_SERVER
 * and providing a list of time servers */
#ifndef SNTPD_USER_TIME_SERVER
#define	SNTPD_USER_TIME_SERVER 0
static const char* default_time_servers[] = {
	"0.pool.ntp.org",
	"1.pool.ntp.org",
	"2.pool.ntp.org",
	"3.pool.ntp.org"
};
#endif

#define EPOCH ( 2208988800UL )

/**
 * Leap Indicator (LI): This is a two-bit code warning of an impending
 * leap second to be inserted/deleted in the last minute of the current
 * day.  This field is significant only in server messages, where the
 * values are defined as follows:
 *
 * 	   LI       Meaning
 *     ---------------------------------------------
 *     0        no warning
 *     1        last minute has 61 seconds
 *     2        last minute has 59 seconds
 *     3        alarm condition (clock not synchronized)
 */
#define SNTPD_FLAGS_LI_NO_WARNING				0x00
#define SNTPD_FLAGS_LI_LAST_MINUTE_HAS_61S		0x40
#define SNTPD_FLAGS_LI_LAST_MINUTE_HAS_59S		0x80
#define SNTPD_FLAGS_LI_NOT_SYNCHRONIZED			0xC0

/**
 * Version Number (VN): This is a three-bit integer indicating the
 * NTP/SNTP version number, currently 4.  If necessary to distinguish
 * between IPv4, IPv6, and OSI, the encapsulating context must be
 * inspected.
 */
#define SNTPD_FLAGS_VN_4	0x20

/**
 * Mode: This is a three-bit number indicating the protocol mode.  The
 * values are defined as follows:
 *
 *     Mode     Meaning
 *     ------------------------------------
 *     0        reserved
 *     1        symmetric active
 *     2        symmetric passive
 *     3        client
 *     4        server
 *     5        broadcast
 *     6        reserved for NTP control message
 *     7        reserved for private use
 */
#define SNTPD_FLAGS_MODE_RESERVED			0x00
#define SNTPD_FLAGS_MODE_SYMMETRIC_ACTIVE	0x01
#define SNTPD_FLAGS_MODE_SYMMETRIC_PASSIVE	0x02
#define SNTPD_FLAGS_MODE_CLIENT				0x03
#define SNTPD_FLAGS_MODE_SERVER				0x04
#define SNTPD_FLAGS_MODE_BROADCAST			0x05
#define SNTPD_FLAGS_MODE_RESERVED_NTP_CTRL	0x06
#define SNTPD_FLAGS_MODE_RESERVED_PRV		0x07

/*
 * Stratum: This is an eight-bit unsigned integer indicating the
 * stratum.  This field is significant only in SNTP server messages,
 * where the values are defined as follows:
 *
 *     Stratum  Meaning
 *     ----------------------------------------------
 *     0        kiss-o'-death message (see below)
 *     1        primary reference (e.g., synchronized by radio clock)
 *     2-15     secondary reference (synchronized by NTP or SNTP)
 *     16-255   reserved
 */
#define SNTPD_STRATUM_KISS_OF_DEATH			0x00

/*
 * Poll Interval: This is an eight-bit unsigned integer used as an
 * exponent of two, where the resulting value is the maximum interval
 * between successive messages in seconds.  This field is significant
 * only in SNTP server messages, where the values range from 4 (16 s) to
 * 17 (131,072 s -- about 36 h).
 */

/*
 * Precision: This is an eight-bit signed integer used as an exponent of
 * two, where the resulting value is the precision of the system clock
 * in seconds.  This field is significant only in server messages, where
 * the values range from -6 for mains-frequency clocks to -20 for
 * microsecond clocks found in some workstations.
 */

/*
 * Root Delay: This is a 32-bit signed fixed-point number indicating the
 * total roundtrip delay to the primary reference source, in seconds
 * with the fraction point between bits 15 and 16.  Note that this
 * variable can take on both positive and negative values, depending on
 * the relative time and frequency offsets.  This field is significant
 * only in server messages, where the values range from negative values
 * of a few milliseconds to positive values of several hundred
 * milliseconds.
 */

/*
 * Root Dispersion: This is a 32-bit unsigned fixed-point number
 * indicating the maximum error due to the clock frequency tolerance, in
 * seconds with the fraction point between bits 15 and 16.  This field
 * is significant only in server messages, where the values range from
 * zero to several hundred milliseconds.
 */

/*
 * Reference Identifier: This is a 32-bit bitstring identifying the
 * particular reference source.  This field is significant only in
 * server messages, where for stratum 0 (kiss-o'-death message) and 1
 * (primary server), the value is a four-character ASCII string, left
 * justified and zero padded to 32 bits.  For IPv4 secondary servers,
 * the value is the 32-bit IPv4 address of the synchronization source.
 * For IPv6 and OSI secondary servers, the value is the first 32 bits of
 * the MD5 hash of the IPv6 or NSAP address of the synchronization
 * source.
 */

/*
 * Reference Timestamp: This field is the time the system clock was last
 * set or corrected, in 64-bit timestamp format.
 */

/*
 * Originate Timestamp: This is the time at which the request departed
 * the client for the server, in 64-bit timestamp format.
 */

/*
 * Receive Timestamp: This is the time at which the request arrived at
 * the server or the reply arrived at the client, in 64-bit timestamp
 * format.
 */

/*
 * Transmit Timestamp: This is the time at which the request departed
 * the client or the reply departed the server, in 64-bit timestamp
 * format.
 */

/*
 * Authenticator (optional): When the NTP authentication scheme is
 * implemented, the Key Identifier and Message Digest fields contain the
 * message authentication code (MAC) information defined in Appendix C
 * of RFC 1305.
 */

/**
 * Create SNTP task to update rtos_clock
 */
void sntp_create( UBaseType_t priority );

/**
 *  Check if time has been synced since last power cycle
 *
 *  \returns	 1 if sync has occurred
 *  			 0 otherwise
 */
int is_time_synced( void );

#endif /* SRC_SNTPD_SNTPD_H_ */
