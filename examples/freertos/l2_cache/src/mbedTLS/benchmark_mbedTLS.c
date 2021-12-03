// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xcore/hwtimer.h>

#include "mbedtls/cipher.h"
#include "mbedtls/md.h"
#include "mbedtls/platform.h"
#include "mbedtls/platform_util.h"

#include "benchmark_mbedTLS.h"
#include "print_info.h"

// NOTE: Origin of this example
//      https://github.com/ARMmbed/mbedtls/blob/development/programs/aes/crypt_and_hash.c

#define CIPHER "AES-128-CBC"
#define MESSAGE_DIGEST "SHA1"
#define NUM_BLOCKS (16)
#define BLOCK_SIZE_BYTES (16)
#define HASH_ITERATIONS (256) //NOTE: is 8192 in the example code at the link above

static const mbedtls_cipher_info_t *cipher_info = NULL;
static const mbedtls_md_info_t *md_info = NULL;
static mbedtls_cipher_context_t cipher_ctx;
static mbedtls_md_context_t md_ctx;

static int8_t IV[16] = {0xa2, 0x78, 0xc9, 0xa4, 0xd8, 0x34, 0x88, 0x9b,
                        0x28, 0xdc, 0xb9, 0xe2, 0xc0, 0x58, 0x8c, 0xbc};
static int8_t key[32] = {0x15, 0x1,  0xc0, 0xd0, 0xe4, 0xfd, 0xdf, 0xd7,
                         0x7a, 0x65, 0xf1, 0x2f, 0x45, 0x61, 0xb,  0x59,
                         0xd9, 0xa,  0x9c, 0x61, 0xc,  0x4,  0x76, 0xdb,
                         0xb,  0xbe, 0x9e, 0xe4, 0x7f, 0x8d, 0xe1, 0x46};
static int8_t encrypted_blocks[NUM_BLOCKS][BLOCK_SIZE_BYTES] = {{0}, {0}};
static int8_t ciphertext[16] = {0};

static size_t key_len = 32;
static size_t msg_len = 16;

static int aes_init(const char *cipher, const char *message_digest) {
  mbedtls_cipher_init(&cipher_ctx);
  mbedtls_md_init(&md_ctx);

  cipher_info = mbedtls_cipher_info_from_string(cipher);
  if (cipher_info == NULL) {
    debug_printf("Cipher '%s' not found\n", cipher);
    return MBEDTLS_EXIT_FAILURE;
  }
  if (mbedtls_cipher_setup(&cipher_ctx, cipher_info) != 0) {
    debug_printf("mbedtls_cipher_setup failed\n");
    return MBEDTLS_EXIT_FAILURE;
  }

  md_info = mbedtls_md_info_from_string(message_digest);
  if (md_info == NULL) {
    debug_printf("Message Digest '%s' not found\n", message_digest);
    return MBEDTLS_EXIT_FAILURE;
  }

  if (mbedtls_md_setup(&md_ctx, md_info, 1) != 0) {
    debug_printf("mbedtls_md_setup failed\n");
    return MBEDTLS_EXIT_FAILURE;
  }

  return MBEDTLS_EXIT_SUCCESS;
}

static void aes_deinit() {
  mbedtls_cipher_free(&cipher_ctx);
  mbedtls_md_free(&md_ctx);
}

static int aes_encrypt(const int8_t *message) {
  size_t olen;
  unsigned char digest[MBEDTLS_MD_MAX_SIZE];
  unsigned char buffer[1024];
  unsigned char output[1024];

  memset(digest, 0, MBEDTLS_MD_MAX_SIZE);
  memcpy(digest, IV, 16);

  for (int i = 0; i < HASH_ITERATIONS; i++) {
    mbedtls_md_starts(&md_ctx);
    mbedtls_md_update(&md_ctx, digest, 32);
    mbedtls_md_update(&md_ctx, (const unsigned char *)&key[0], key_len);
    mbedtls_md_finish(&md_ctx, digest);
  }

  if (mbedtls_cipher_setkey(&cipher_ctx, digest, (int)cipher_info->key_bitlen,
                            MBEDTLS_ENCRYPT) != 0) {
    debug_printf("mbedtls_cipher_setkey() returned error\n");
    return MBEDTLS_EXIT_FAILURE;
  }
  if (mbedtls_cipher_set_iv(&cipher_ctx, (const unsigned char *)&IV[0], 16) !=
      0) {
    debug_printf("mbedtls_cipher_set_iv() returned error\n");
    return MBEDTLS_EXIT_FAILURE;
  }
  if (mbedtls_cipher_reset(&cipher_ctx) != 0) {
    debug_printf("mbedtls_cipher_reset() returned error\n");
    return MBEDTLS_EXIT_FAILURE;
  }

  mbedtls_md_hmac_starts(&md_ctx, digest, 32);

  for (int i = 0; i < NUM_BLOCKS; i++) {
    memcpy(buffer, message, msg_len);

    if (mbedtls_cipher_update(&cipher_ctx, buffer, BLOCK_SIZE_BYTES, output,
                              &olen) != 0) {
      debug_printf("mbedtls_cipher_update() returned error\n");
      return MBEDTLS_EXIT_FAILURE;
    }

    mbedtls_md_hmac_update(&md_ctx, output, olen);
    memcpy(&encrypted_blocks[i][0], output, BLOCK_SIZE_BYTES);
  }

  if (mbedtls_cipher_finish(&cipher_ctx, output, &olen) != 0) {
    debug_printf("mbedtls_cipher_finish() returned error\n");
    return MBEDTLS_EXIT_FAILURE;
  }
  mbedtls_md_hmac_update(&md_ctx, output, olen);

  mbedtls_md_hmac_finish(&md_ctx, digest);

  memcpy(&ciphertext[0], (const unsigned char *)&digest[0], 16);

  return MBEDTLS_EXIT_SUCCESS;
}

static int aes_decrypt(int8_t *plaintext) {
  size_t olen;
  unsigned char digest[MBEDTLS_MD_MAX_SIZE];
  unsigned char buffer[1024];
  unsigned char output[1024];

  memset(digest, 0, 32);
  memcpy(digest, IV, 16);

  for (int i = 0; i < HASH_ITERATIONS; i++) {
    mbedtls_md_starts(&md_ctx);
    mbedtls_md_update(&md_ctx, digest, 32);
    mbedtls_md_update(&md_ctx, (const unsigned char *)&key[0], key_len);
    mbedtls_md_finish(&md_ctx, digest);
  }

  if (mbedtls_cipher_setkey(&cipher_ctx, digest, (int)cipher_info->key_bitlen,
                            MBEDTLS_DECRYPT) != 0) {
    debug_printf("mbedtls_cipher_setkey() returned error\n");
    return MBEDTLS_EXIT_FAILURE;
  }
  if (mbedtls_cipher_set_iv(&cipher_ctx, (const unsigned char *)&IV[0], 16) !=
      0) {
    debug_printf("mbedtls_cipher_set_iv() returned error\n");
    return MBEDTLS_EXIT_FAILURE;
  }
  if (mbedtls_cipher_reset(&cipher_ctx) != 0) {
    debug_printf("mbedtls_cipher_reset() returned error\n");
    return MBEDTLS_EXIT_FAILURE;
  }

  mbedtls_md_hmac_starts(&md_ctx, digest, 32);

  for (int i = 0; i < NUM_BLOCKS; i++) {
    memcpy(buffer, &encrypted_blocks[i][0], BLOCK_SIZE_BYTES);

    mbedtls_md_hmac_update(&md_ctx, buffer, BLOCK_SIZE_BYTES);

    if (mbedtls_cipher_update(&cipher_ctx, buffer, BLOCK_SIZE_BYTES, output,
                              &olen) != 0) {
      debug_printf("mbedtls_cipher_update() returned error\n");
      return MBEDTLS_EXIT_FAILURE;
    }
  }

  mbedtls_md_hmac_finish(&md_ctx, digest);

  mbedtls_cipher_finish(&cipher_ctx, output, &olen);

  memcpy(plaintext, &output[0], msg_len);

  return MBEDTLS_EXIT_SUCCESS;
}

int aes_crypt_and_hash() {
  int8_t message[16] = {0x3b, 0xb1, 0x99, 0x3,  0x67, 0xf3, 0x2e, 0x1f,
                        0x00, 0x67, 0x38, 0xc9, 0x53, 0x92, 0xa4};
  int8_t plaintext[16] = {0};

  if (aes_init(CIPHER, MESSAGE_DIGEST) != MBEDTLS_EXIT_SUCCESS) {
    return MBEDTLS_EXIT_FAILURE;
  }

  if (aes_encrypt(&message[0]) != MBEDTLS_EXIT_SUCCESS) {
    return MBEDTLS_EXIT_FAILURE;
  }

  if (aes_decrypt(&plaintext[0]) != MBEDTLS_EXIT_SUCCESS) {
    return MBEDTLS_EXIT_FAILURE;
  }

  // verify the plain text against the message
  for (int i = 0; i < msg_len; i++) {
    // debug_printf("0x%x\n", plaintext[i]);
    if (message[i] != plaintext[i]) {
      return MBEDTLS_EXIT_FAILURE;
    }
  }

  aes_deinit();

  return MBEDTLS_EXIT_SUCCESS;
}

#pragma stackfunction 1200
void benchmark_aes() {
  uint32_t elapsed_time;
  int retval;

  debug_printf("\nAES Crypt & Hash\n");
  elapsed_time = get_reference_time();
  retval = aes_crypt_and_hash();
  elapsed_time = get_reference_time() - elapsed_time;
  assert(retval == MBEDTLS_EXIT_SUCCESS);

  print_info(elapsed_time);
}
