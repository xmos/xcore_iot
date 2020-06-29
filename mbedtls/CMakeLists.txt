#=============================================================================
# Library creator shall specify the following variables:
#   LIB_NAME                Name of the library
#                           May only contain alphanumeric and space, dash, and
#                           underscore characters.
#                           ex: set(LIB_NAME lib_foo)
#   LIB_VERSION             Version of the library
#                           Must follow semantic version format.
#                           ex: set(LIB_VERSION 1.0.0)
#   LIB_ADD_COMPILER_FLAGS  Additional compiler flags
#                           ex: set(LIB_ADD_COMPILER_FLAGS "-oS")
#   LIB_XC_SRCS             List of all library XC sources
#                           Shall be set to "" if no XC sources exist
#                           ex: set(LIB_XC_SRCS "foo.xc" "bar.xc")
#   LIB_C_SRCS              List of all library C sources
#                           Shall be set to "" if no C sources exist
#                           ex: set(LIB_C_SRCS "foo.c" "bar.c")
#   LIB_ASM_SRCS            List of all library ASM sources
#                           Shall be set to "" if no ASM sources exist
#                           ex: set(LIB_ASM_SRCS "foo.S" "bar.S")
#   LIB_INCLUDES            List of all library include directories
#                           Shall be set to all directories include .h files
#                           ex: set(LIB_INCLUDES "foo/bar")
#   LIB_DEPENDENT_MODULES   List of all dependency libraries with version
#                           Every list item shall include a dependency name,
#                           which must also be the folder in which the library
#                           CMakeLists.txt exists, and a version requirement.
#                           Version requirements shall be in the format:
#                           ([bool req][major].[minor].[patch])
#                           ex: set(LIB_DEPENDENT_MODULES "lib_foo(>=1.0.0)")
#   LIB_OPTIONAL_HEADERS    List of optional header files
#                           Must contain full header file names
#                           ex: set(LIB_OPTIONAL_HEADERS "foo.h" "bar.h")
set(LIB_NAME mbedtls)
set(LIB_VERSION 2.16.6)
set(LIB_ADD_COMPILER_FLAGS "")
set(LIB_XC_SRCS
        ""
    )
set(LIB_C_SRCS
        "mbedtls/library/aes.c"
        "mbedtls/library/aesni.c"
        "mbedtls/library/arc4.c"
        "mbedtls/library/aria.c"
        "mbedtls/library/asn1parse.c"
        "mbedtls/library/asn1write.c"
        "mbedtls/library/base64.c"
        "mbedtls/library/bignum.c"
        "mbedtls/library/blowfish.c"
        "mbedtls/library/camellia.c"
        "mbedtls/library/ccm.c"
        "mbedtls/library/chacha20.c"
        "mbedtls/library/chachapoly.c"
        "mbedtls/library/cipher.c"
        "mbedtls/library/cipher_wrap.c"
        "mbedtls/library/cmac.c"
        "mbedtls/library/ctr_drbg.c"
        "mbedtls/library/des.c"
        "mbedtls/library/dhm.c"
        "mbedtls/library/ecdh.c"
        "mbedtls/library/ecdsa.c"
        "mbedtls/library/ecjpake.c"
        "mbedtls/library/ecp.c"
        "mbedtls/library/ecp_curves.c"
        "mbedtls/library/entropy.c"
        "mbedtls/library/entropy_poll.c"
        "mbedtls/library/error.c"
        "mbedtls/library/gcm.c"
        "mbedtls/library/havege.c"
        "mbedtls/library/hkdf.c"
        "mbedtls/library/hmac_drbg.c"
        "mbedtls/library/md.c"
        "mbedtls/library/md2.c"
        "mbedtls/library/md4.c"
        "mbedtls/library/md5.c"
        "mbedtls/library/md_wrap.c"
        "mbedtls/library/memory_buffer_alloc.c"
        "mbedtls/library/nist_kw.c"
        "mbedtls/library/oid.c"
        "mbedtls/library/padlock.c"
        "mbedtls/library/pem.c"
        "mbedtls/library/pk.c"
        "mbedtls/library/pk_wrap.c"
        "mbedtls/library/pkcs12.c"
        "mbedtls/library/pkcs5.c"
        "mbedtls/library/pkparse.c"
        "mbedtls/library/pkwrite.c"
        "mbedtls/library/platform.c"
        "mbedtls/library/platform_util.c"
        "mbedtls/library/poly1305.c"
        "mbedtls/library/ripemd160.c"
        "mbedtls/library/rsa.c"
        "mbedtls/library/rsa_internal.c"
        "mbedtls/library/sha1.c"
        "mbedtls/library/sha256.c"
        "mbedtls/library/sha512.c"
        "mbedtls/library/threading.c"
        "mbedtls/library/timing.c"
        "mbedtls/library/version.c"
        "mbedtls/library/version_features.c"
        "mbedtls/library/xtea.c"
        "mbedtls/library/certs.c"
        "mbedtls/library/pkcs11.c"
        "mbedtls/library/x509.c"
        "mbedtls/library/x509_create.c"
        "mbedtls/library/x509_crl.c"
        "mbedtls/library/x509_crt.c"
        "mbedtls/library/x509_csr.c"
        "mbedtls/library/x509write_crt.c"
        "mbedtls/library/x509write_csr.c"
        "mbedtls/library/debug.c"
        "mbedtls/library/net_sockets.c"
        "mbedtls/library/ssl_cache.c"
        "mbedtls/library/ssl_ciphersuites.c"
        "mbedtls/library/ssl_cli.c"
        "mbedtls/library/ssl_cookie.c"
        "mbedtls/library/ssl_srv.c"
        "mbedtls/library/ssl_ticket.c"
        "mbedtls/library/ssl_tls.c"
        "port/mbedtls"
    )
set(LIB_ASM_SRCS
        ""
    )
set(LIB_INCLUDES
        "mbedtls/include"
        "port/mbedtls"
    )
set(LIB_DEPENDENT_MODULES "lib_rtos_support(>=0.5.0)")
set(LIB_OPTIONAL_HEADERS "")

set(${LIB_NAME}_FILEPATH_PREFIX "${XMOS_MODULES_ROOT_DIR}/thirdparty/")

list(TRANSFORM LIB_XC_SRCS PREPEND ${${LIB_NAME}_FILEPATH_PREFIX})
list(TRANSFORM LIB_C_SRCS PREPEND ${${LIB_NAME}_FILEPATH_PREFIX})
list(TRANSFORM LIB_ASM_SRCS PREPEND ${${LIB_NAME}_FILEPATH_PREFIX})
list(TRANSFORM LIB_INCLUDES PREPEND ${${LIB_NAME}_FILEPATH_PREFIX})

# #=============================================================================
# # The following part of the CMakeLists includes the common build infrastructure
# # for compiling XMOS applications. You should not need to edit below here.
XMOS_REGISTER_MODULE()
