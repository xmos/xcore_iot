// Copyright 2016 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

//#define DEBUG_UNIT CONTROL
#include <rtos_printf.h>

#include "device_control.h"

static control_resid_t requested_resid;
static control_cmd_t requested_cmd;
static size_t requested_payload_len;

void resource_table_init(control_resid_t reserved_id);

int resource_table_add(const control_resid_t resources[],
                       size_t num_resources,
                       uint8_t ifnum);

int resource_table_search(control_resid_t resid, uint8_t *ifnum);


static control_ret_t special_read_command(control_cmd_t cmd,
                                          uint8_t payload[],
                                          unsigned payload_len)
{
    if (cmd == CONTROL_GET_VERSION) {
        rtos_printf("read version\n");
        if (payload_len != sizeof(control_version_t)) {
            rtos_printf("wrong payload size %d for read version command, need %d\n",
                    payload_len, sizeof(control_version_t));

            return CONTROL_BAD_COMMAND;
        } else {
            *((control_version_t*) payload) = CONTROL_VERSION;
            return CONTROL_SUCCESS;
        }
    } else {
        rtos_printf("unrecognised special resource command %d\n", cmd);
        return CONTROL_BAD_COMMAND;
    }
}

static control_ret_t write_command(int ifnum,
                                   control_resid_t resid,
                                   control_cmd_t cmd,
                                   const uint8_t payload[],
                                   unsigned payload_len)
{
    if (resid == CONTROL_SPECIAL_RESID) {
        rtos_printf("ignoring write to special resource %d\n", CONTROL_SPECIAL_RESID);
        return CONTROL_BAD_COMMAND;
    } else {
        rtos_printf("%d write command %d, %d, %d\n", ifnum, resid, cmd, payload_len);
        //control_ret_t ret = i[ifnum].write_command(resid, cmd, payload, payload_len);
        control_ret_t ret = CONTROL_SUCCESS;
        return ret;
    }
}

static control_ret_t read_command(unsigned char ifnum,
                                  control_resid_t resid,
                                  control_cmd_t cmd,
                                  uint8_t payload[],
                                  unsigned payload_len)
{
    if (resid == CONTROL_SPECIAL_RESID) {
        return special_read_command(cmd, payload, payload_len);
    } else {
        rtos_printf("%d read command %d, %d, %d\n", ifnum, resid, cmd, payload_len);
        //control_ret_t ret = i[ifnum].read_command(resid, cmd, payload, payload_len);
        control_ret_t ret = CONTROL_SUCCESS;
        return ret;
    }
}



void device_control_request(control_resid_t resid,
                            control_cmd_t cmd,
                            size_t payload_len)
{
    requested_resid = resid;
    requested_cmd = cmd;
    requested_payload_len = payload_len;
}

control_ret_t device_control_payload_transfer(uint8_t *payload_buf,
                                              size_t buf_size,
                                              control_direction_t direction)
{
    uint8_t ifnum;

    if (resource_table_search(requested_resid, &ifnum) != 0) {
        rtos_printf("resource %d not found\n", requested_resid);
        return CONTROL_BAD_COMMAND;
    }

    if (requested_payload_len > buf_size) {
        return CONTROL_DATA_LENGTH_ERROR;
    }

    if (direction == CONTROL_DEVICE_TO_HOST && IS_CONTROL_CMD_READ(requested_cmd)) {
        return read_command(ifnum, requested_resid, requested_cmd, payload_buf, requested_payload_len);
    } else if (direction == CONTROL_HOST_TO_DEVICE && !IS_CONTROL_CMD_READ(requested_cmd)) {
        return write_command(ifnum, requested_resid, requested_cmd, payload_buf, requested_payload_len);
    } else {
        if (buf_size > 0) {
            return CONTROL_BAD_COMMAND;
        } else {
            return CONTROL_SUCCESS;
        }
    }
}

control_ret_t control_init(void)
{
  resource_table_init(CONTROL_SPECIAL_RESID);
  return CONTROL_SUCCESS;
}

