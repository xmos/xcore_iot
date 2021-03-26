// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef DEVICE_CONTROL_H_
#define DEVICE_CONTROL_H_

#include <stdint.h>
#include <stddef.h>

/**
 * This is the version of control protocol. Used to check compatibility
 */
#define CONTROL_VERSION 0x10

/**
 * These types are used in control functions to identify the resource id,
 * command, and version.
 */
typedef uint8_t control_resid_t;
typedef uint8_t control_cmd_t;
typedef uint8_t control_version_t;

/**
 * This type enumerates the possible outcomes from a control transaction.
 */
typedef enum {
    CONTROL_SUCCESS = 0,
    CONTROL_REGISTRATION_FAILED,
    CONTROL_BAD_COMMAND,
    CONTROL_DATA_LENGTH_ERROR,
    CONTROL_OTHER_TRANSPORT_ERROR,
    CONTROL_ERROR
} control_ret_t;

typedef enum {
    CONTROL_HOST_TO_DEVICE,
    CONTROL_DEVICE_TO_HOST
} control_direction_t;

/**
 * Resource count limits. Sets the size of the arrays used for storing the mappings
 */
#define RESOURCE_TABLE_MAX 32
#define IFNUM_RESERVED (RESOURCE_TABLE_MAX - 1)

#define IS_CONTROL_CMD_READ(c) ((c) & 0x80)
#define CONTROL_CMD_SET_READ(c) ((c) | 0x80)
#define CONTROL_CMD_SET_WRITE(c) ((c) & ~0x80)

#define CONTROL_SPECIAL_RESID 0

#define CONTROL_GET_VERSION CONTROL_CMD_SET_READ(0)
#define CONTROL_GET_LAST_COMMAND_STATUS CONTROL_CMD_SET_READ(1)

void device_control_request(control_resid_t resid,
                            control_cmd_t cmd,
                            size_t payload_len);

control_ret_t device_control_payload_transfer(uint8_t *payload_buf,
                                              size_t buf_size,
                                              control_direction_t direction);

/**
 * Initiaize the control library. Clears resource table to ensure nothing is registered.
 *
 * \returns  Whether the initialization was successful or not
 */
control_ret_t control_init(void);

#endif /* DEVICE_CONTROL_H_ */
