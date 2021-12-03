set(MBEDTLS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/mbedTLS")

## Source files
file(GLOB_RECURSE MBEDTLS_SOURCES  "${MBEDTLS_PATH}/*.c" "${MBEDTLS_PATH}/library/*.c")

## Includes
set(MBEDTLS_INCLUDES "${MBEDTLS_PATH}" "${MBEDTLS_PATH}/include")

add_compile_definitions(
    MBEDTLS_CONFIG_FILE=\"${MBEDTLS_PATH}/mbedtls_xcore_default_config.h\"
)
