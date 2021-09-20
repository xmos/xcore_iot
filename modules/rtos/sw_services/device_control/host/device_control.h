// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef DEVICE_CONTROL_H_
#define DEVICE_CONTROL_H_

#include <stdint.h>

/**
 * This is the version of control protocol. Used to check compatibility
 */
#define CONTROL_VERSION 0x10

/**
 * @{
 * These types are used in control functions to identify the resource id,
 * command, version, and status.
 */
typedef uint8_t control_resid_t;
typedef uint8_t control_cmd_t;
typedef uint8_t control_version_t;
typedef uint8_t control_status_t;
/**@}*/

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

/**
 * This type is used to inform the control library the direction of
 * a control transfer from the transport layer.
 */
typedef enum {
    CONTROL_HOST_TO_DEVICE,
    CONTROL_DEVICE_TO_HOST
} control_direction_t;

/**
 * Checks if the read bit is set in a command code.
 *
 * \param[in] c The command code to check
 *
 * \returns true if the read bit in the command is set
 * \returns false if the read bit is not set
 */
#define IS_CONTROL_CMD_READ(c) ((c) & 0x80)

/**
 * Sets the read bit on a command code
 *
 * \param[in,out] c The command code to set the read bit on.
 */
#define CONTROL_CMD_SET_READ(c) ((c) | 0x80)

/**
 * Clears the read bit on a command code
 *
 * \param[in,out] c The command code to clear the read bit on.
 */
#define CONTROL_CMD_SET_WRITE(c) ((c) & ~0x80)

/**
 * This is the special resource ID owned by the control library.
 * It can be used to check the version of the control protocol.
 * Servicers may not register this resource ID.
 */
#define CONTROL_SPECIAL_RESID 0

/**
 * The maximum resource ID. IDs greater than this cannot
 * be registered.
 */
#define CONTROL_MAX_RESOURCE_ID 255

/**
 * The command to read the version of the control protocol.
 * It must be sent to resource ID CONTROL_SPECIAL_RESID.
 */
#define CONTROL_GET_VERSION CONTROL_CMD_SET_READ(0)

/**
 * The command to read the return status of the last command.
 * It must be sent to resource ID CONTROL_SPECIAL_RESID.
 */
#define CONTROL_GET_LAST_COMMAND_STATUS CONTROL_CMD_SET_READ(1)


#endif /* DEVICE_CONTROL_H_ */
