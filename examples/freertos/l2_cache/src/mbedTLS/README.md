# Introduction

This code is a copy of the mbedTLS sources in the XCore SDK, at commit 4c5c556. It has been modified to run from flash by
adding:  

    XCORE_CODE_SECTION_ATTRIBUTE

To the following code symbols:

     mbedtls_aes_init
     mbedtls_aes_free
     mbedtls_aes_xts_init
     mbedtls_aes_xts_free
     mbedtls_aes_setkey_enc
     mbedtls_aes_setkey_dec
     mbedtls_aes_xts_setkey_enc
     mbedtls_aes_xts_setkey_dec
     mbedtls_internal_aes_encrypt
     mbedtls_internal_aes_decrypt
     mbedtls_aes_crypt_ecb
     mbedtls_aes_crypt_cbc
     mbedtls_aes_crypt_xts
     mbedtls_gf128mul_x_ble
     mbedtls_aes_crypt_cfb128
     mbedtls_aes_crypt_ofb
     mbedtls_aes_crypt_ctr
     mbedtls_cipher_info_from_string
     mbedtls_cipher_init
     mbedtls_cipher_free
     mbedtls_cipher_setup
     mbedtls_cipher_setkey
     mbedtls_cipher_set_iv
     mbedtls_cipher_reset
     mbedtls_cipher_update
     mbedtls_cipher_finish
     mbedtls_md_info_from_string
     mbedtls_md_init
     mbedtls_md_free
     mbedtls_md_setup
     mbedtls_md_starts
     mbedtls_md_update
     mbedtls_md_finish
     mbedtls_md_hmac_starts
     mbedtls_md_hmac_update
     mbedtls_md_hmac_finish
     mbedtls_platform_zeroize
     mbedtls_sha1_init
     mbedtls_sha1_free
     mbedtls_sha1_clone
     mbedtls_sha1_starts_ret
     mbedtls_internal_sha1_process
     mbedtls_sha1_update_ret
     mbedtls_sha1_finish_ret
     mbedtls_sha1_ret
     mbedtls_sha256_init
     mbedtls_sha256_free
     mbedtls_sha256_clone
     mbedtls_sha256_starts_ret
     mbedtls_internal_sha256_process
     mbedtls_sha256_update_ret
     mbedtls_sha256_finish_ret
     mbedtls_sha256_ret
     mbedtls_sha512_init
     mbedtls_sha512_free
     mbedtls_sha512_clone
     mbedtls_sha512_starts_ret
     mbedtls_internal_sha512_process
     mbedtls_sha512_update_ret
     mbedtls_sha512_finish_ret
     mbedtls_sha512_ret

And adding:

    #include "swmem_macros.h"

to the top of each file containing these symbols.
