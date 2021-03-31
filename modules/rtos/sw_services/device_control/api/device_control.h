// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef DEVICE_CONTROL_H_
#define DEVICE_CONTROL_H_

#include <stdint.h>
#include <stddef.h>

#include "rtos/drivers/osal/api/rtos_osal.h"
#include "rtos/drivers/intertile/api/rtos_intertile.h"

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


#define IS_CONTROL_CMD_READ(c) ((c) & 0x80)
#define CONTROL_CMD_SET_READ(c) ((c) | 0x80)
#define CONTROL_CMD_SET_WRITE(c) ((c) & ~0x80)

#define CONTROL_SPECIAL_RESID 0
#define CONTROL_MAX_RESOURCE_ID 255

#define CONTROL_GET_VERSION CONTROL_CMD_SET_READ(0)
#define CONTROL_GET_LAST_COMMAND_STATUS CONTROL_CMD_SET_READ(1)

typedef struct {
    rtos_osal_queue_t gateway_queue;
    union {
        rtos_intertile_t *client_intertile[3];
        rtos_intertile_t *host_intertile;
    };
    size_t intertile_count;
    int intertile_port;
    uint8_t *resource_table; /* NULL on client tiles */
    //device_control_servicer_t *servicer_table;

    struct {
        rtos_intertile_t *intertile_ctx;
        rtos_osal_queue_t *queue;
    } *servicer_table;

    size_t requested_payload_len;
    control_resid_t requested_resid;
    control_cmd_t requested_cmd;
} device_control_t;

typedef struct {
    device_control_t *device_control_ctx;
    rtos_osal_queue_t queue;
} device_control_servicer_t;

void device_control_request(device_control_t *ctx,
                            control_resid_t resid,
                            control_cmd_t cmd,
                            size_t payload_len);

control_ret_t device_control_payload_transfer(device_control_t *ctx,
                                              uint8_t *payload_buf,
                                              size_t buf_size,
                                              control_direction_t direction);

#define DEVICE_CONTROL_HOST_MODE   0
#define DEVICE_CONTROL_CLIENT_MODE 1



/**
 * \param servicer_count  The number of "servicers". Each servicer must register before
 *                        this function can return.
 */
control_ret_t device_control_resources_register(device_control_t *ctx,
                                                const size_t servicer_count,
                                                unsigned timeout);

control_ret_t device_control_servicer_register(device_control_servicer_t *ctx,
                                               device_control_t *device_control_ctx,
                                               const control_resid_t resources[],
                                               size_t num_resources);

control_ret_t device_control_start(device_control_t *ctx,
                                   uint8_t intertile_port,
                                   unsigned priority);

/**
 * Initiaize the control library. Clears resource table to ensure nothing is registered.
 *
 * Must be run in an RTOS thread prior to receiving commands from the transport.
 *
 * \param ctx             The device control context to initialize.
 * \param mode            Set to DEVICE_CONTROL_HOST_MODE if the command transport is on the
 *                        same tile. Set to DEVICE_CONTROL_CLIENT_MODE if the command transport
 *                        is on another tile.
 * \param intertile_ctx   An array of intertile contexts used to communicate with other tiles.
 * \param intertile_count The number of intertile contexts in the \p intertile_ctx array.
 *                        When \p mode is DEVICE_CONTROL_HOST_MODE, this may be 0 if there are
 *                        no servicers on other tiles, up to one per device control instance that
 *                        has been initialized with DEVICE_CONTROL_HOST_MODE on other tiles.
 *                        When \p mode is DEVICE_CONTROL_CLIENT_MODE then this must be 1,
 *                        and the intertile context must connect to a device control instance
 *                        on another tile that has been initialized with DEVICE_CONTROL_HOST_MODE.
 *
 * \returns               CONTROL_SUCCESS if the initialization was successful. An error status
 *                        otherwise.
 */
control_ret_t device_control_init(device_control_t *ctx,
                                  int mode,
                                  rtos_intertile_t *intertile_ctx[],
                                  size_t intertile_count);

#endif /* DEVICE_CONTROL_H_ */
