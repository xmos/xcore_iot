// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef TLS_SUPPORT_H_
#define TLS_SUPPORT_H_

#define DEVICE_CA_CHAIN_FILEPATH_DEFAULT	"/flash/crypto/ca.pem"
#define DEVICE_CERT_FILEPATH_DEFAULT		"/flash/crypto/cert.pem"
#define DEVICE_PRV_KEY_FILEPATH_DEFAULT		"/flash/crypto/key.pem"

/**
 *  Perform TLS platform required setup
 */
void tls_platform_init( void );

/**
 * Cleanup any TLS platform setup
 */
void tls_platform_free( void );

/**
 *  Check if the TLS platform is ready to use
 *
 *  \returns	 1 if ready
 *  			 0 otherwise
 */
int tls_platform_ready( void );

/**
 * Send to network driver using TLS
 *
 * \param[in]     ctx		     Context to use containing the network,
 * 								 TLS, and any other tls_support information
 * \param[in]     buf            Pointer to the buffer to send
 * \param[in]     len    		 Number of bytes buffer to send
 *
 * \returns       return value of configured network send function
 */
int tls_send( void* ctx, const unsigned char* buf, size_t len);

/**
 * Receive from network driver using TLS
 *
 * \param[in]     ctx		     Context to use containing the network,
 * 								 TLS, and any other tls_support information
 * \param[in/out] buf            Pointer to the buffer to receive into
 * \param[in]     len    		 Maximum number of bytes to read
 *
 * \returns       return value of configured network receive function
 */
int tls_recv( void* ctx, unsigned char* buf, size_t len);

#ifdef DEVICE_CA_CHAIN_FILEPATH
static const char* ca_chain_filepath = DEVICE_CA_CHAIN_FILEPATH;
#else
static const char* ca_chain_filepath = DEVICE_CA_CHAIN_FILEPATH_DEFAULT;
#endif

#ifdef DEVICE_PRV_KEY_FILEPATH
static const char* prvkey_filepath = DEVICE_PRV_KEY_FILEPATH;
#else
static const char* prvkey_filepath = DEVICE_PRV_KEY_FILEPATH_DEFAULT;
#endif

#ifdef DEVICE_CERT_FILEPATH
static const char* cert_filepath = DEVICE_CERT_FILEPATH;
#else
static const char* cert_filepath = DEVICE_CERT_FILEPATH_DEFAULT;
#endif

#if RTOS_FREERTOS
/* Currently only support FreeRTOS with mbedtls */
#include "mbedtls_support.h"
#endif

#endif /* TLS_SUPPORT_H_ */
