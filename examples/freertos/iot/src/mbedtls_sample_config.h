// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* This file provides a default configuration for the mbed TLS library for xcore */

#ifndef MBEDTLS_XCORE_DEFAULT_CONFIG_H
#define MBEDTLS_XCORE_DEFAULT_CONFIG_H

/* System support for assembly and time. */
#define MBEDTLS_HAVE_ASM
#define MBEDTLS_HAVE_TIME
#define MBEDTLS_HAVE_TIME_DATE
#define MBEDTLS_PLATFORM_GMTIME_R_ALT

/* Remove deprecated functions to prevent their use. */
#define MBEDTLS_DEPRECATED_REMOVED

/* Enabled block cipher modes of operation. */
#define MBEDTLS_CIPHER_MODE_CBC
#define MBEDTLS_CIPHER_MODE_CFB
#define MBEDTLS_CIPHER_MODE_CTR
#define MBEDTLS_CIPHER_MODE_OFB
#define MBEDTLS_CIPHER_MODE_XTS

/* Enabled block cipher padding modes. */
#define MBEDTLS_CIPHER_PADDING_ONE_AND_ZEROS
#define MBEDTLS_CIPHER_PADDING_ZEROS_AND_LEN
#define MBEDTLS_CIPHER_PADDING_ZEROS

/* Disable weak cipher suites. */
#define MBEDTLS_REMOVE_ARC4_CIPHERSUITES
#define MBEDTLS_REMOVE_3DES_CIPHERSUITES

/* Enabled eliptic curves. */
#define MBEDTLS_ECP_DP_SECP192R1_ENABLED
#define MBEDTLS_ECP_DP_SECP224R1_ENABLED
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_ECP_DP_SECP384R1_ENABLED
#define MBEDTLS_ECP_DP_SECP521R1_ENABLED
#define MBEDTLS_ECP_DP_SECP192K1_ENABLED
#define MBEDTLS_ECP_DP_SECP224K1_ENABLED
#define MBEDTLS_ECP_DP_SECP256K1_ENABLED
#define MBEDTLS_ECP_DP_BP256R1_ENABLED
#define MBEDTLS_ECP_DP_BP384R1_ENABLED
#define MBEDTLS_ECP_DP_BP512R1_ENABLED
#define MBEDTLS_ECP_DP_CURVE25519_ENABLED
#define MBEDTLS_ECP_DP_CURVE448_ENABLED

/* Enable NIST curves optimization and deterministic ECDSA. */
#define MBEDTLS_ECP_NIST_OPTIM
#define MBEDTLS_ECDSA_DETERMINISTIC

/* Enable TLS cipher suites supported by AWS IoT. */
#define MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED

/* Enable Encrypt-then-MAC and Extended Master Secret. */
#define MBEDTLS_SSL_ENCRYPT_THEN_MAC
#define MBEDTLS_SSL_EXTENDED_MASTER_SECRET

/* Enable hardware acceleration in the SSL module. */
//#define MBEDTLS_SSL_HW_RECORD_ACCEL

/* Enable SSL max fragment length, ALPN, and SNI. */
#define MBEDTLS_SSL_MAX_FRAGMENT_LENGTH
#define MBEDTLS_SSL_ALPN
#define MBEDTLS_SSL_SERVER_NAME_INDICATION

/* Enable TLS v1.2 only. */
#define MBEDTLS_SSL_PROTO_TLS1_2

/* Enable verification of key usage and extended key usage. */
#define MBEDTLS_X509_CHECK_KEY_USAGE
#define MBEDTLS_X509_CHECK_EXTENDED_KEY_USAGE

/* Use x86 AES-NI instructions. */
#define MBEDTLS_AESNI_C

/* Enabled mbed TLS modules. */
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_AES_C
#define MBEDTLS_BASE64_C
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_CIPHER_C
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_ECDSA_C
#define MBEDTLS_ECDH_C
#define MBEDTLS_ECP_C
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_ERROR_C
#define MBEDTLS_HMAC_DRBG_C
#define MBEDTLS_MD_C
#define MBEDTLS_OID_C
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_PKCS1_V15
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_RSA_C
#define MBEDTLS_SHA256_C
#define MBEDTLS_SHA512_C
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_SRV_C
#define MBEDTLS_SSL_TLS_C
#define MBEDTLS_X509_USE_C
#define MBEDTLS_X509_CRT_PARSE_C

/* Enable debug */
//#define MBEDTLS_DEBUG_C

/* Enable platform memory */
#define MBEDTLS_PLATFORM_MEMORY

/* Use platform mutexes in mbed TLS. */
#define MBEDTLS_THREADING_C
#define MBEDTLS_THREADING_ALT

/* Enable platform entropy */
#define MBEDTLS_ENTROPY_HARDWARE_ALT
#define MBEDTLS_NO_PLATFORM_ENTROPY

/**
 * mbed TLS platform macros for xcore
 * @{
 */

/**
 * This ensures that the real <time.h> is never included,
 * which causes problems on xcore as another system header
 * can redefine clock.
 */
#define _TIME_H_

/**
 * Since the real <time.h> doesn't get included, this struct
 * must be defined.
 */
struct tm
{
  int   tm_sec;
  int   tm_min;
  int   tm_hour;
  int   tm_mday;
  int   tm_mon;
  int   tm_year;
  int   tm_wday;
  int   tm_yday;
  int   tm_isdst;
};

/**
 * Since the real <time.h> doesn't get included, time_t
 * does not get defined. mbed TLS does not use time_t
 * directly, but rather mbedtls_time. When this macro
 * is defined, mbedtls_time is typedef'd to it rather
 * than to time_t.
 */
#define MBEDTLS_PLATFORM_TIME_TYPE_MACRO long

/**
 * When this macro is defined, mbedtls_time is defined
 * to it, rather than to the "real" time function in
 * time.h.
 */
#define MBEDTLS_PLATFORM_TIME_MACRO mbedtls_platform_time

/**
 * Tells mbed TLS not to use various standard functions
 * found in the standard C library. This allows us to
 * provide our own versions.
 */
#define MBEDTLS_PLATFORM_NO_STD_FUNCTIONS

/**
 * When the standard C library functions are not used, this
 * sets the include file to include to find the prototypes
 * for our versions.
 */
#define MBEDTLS_PLATFORM_STD_MEM_HDR <mbedtls_xcore_platform.h>

/**
 * Tells mbed TLS to use the FreeRTOS free function
 * vPortFree().
 */
#define MBEDTLS_PLATFORM_FREE_MACRO vPortFree

/**
 * Tells mbed TLS to use our version of calloc, freertos_calloc(),
 * which is a wrapper around pvPortMalloc() and mbedtls_platform_zeroize().
 */
#define MBEDTLS_PLATFORM_CALLOC_MACRO freertos_calloc

/**
 * We do not support fprintf, so remove it completely.
 */
#define MBEDTLS_PLATFORM_FPRINTF_MACRO()

/**
 * Tell mbed TLS to use rtos_printf() instead of printf.
 */
#define MBEDTLS_PLATFORM_PRINTF_MACRO     rtos_printf

/**
 * Tell mbed TLS to use rtos_snprintf() instead of snprintf.
 */
#define MBEDTLS_PLATFORM_SNPRINTF_MACRO   rtos_snprintf

/**@}*/

/* Validate mbed TLS configuration. */
#include "mbedtls/check_config.h"

#endif /* MBEDTLS_XCORE_DEFAULT_CONFIG_H */
