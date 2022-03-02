/*
 * commands.c
 *
 *  Created on: Aug 27, 2021
 *      Author: mbruno
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "commands.h"

/* TODO: Include a header shared with the firmware that has all the resource and command IDs */
//#include "app_control.h"
#define APP_CONTROL_RESID_AP 1
#define APP_CONTROL_RESID_AEC 2
#define APP_CONTROL_RESID_STAGE1 3
#define APP_CONTROL_RESID_STAGE2 4

/*
 * All commands here should have MSB set to 0.
 * If the command is sent as a read command then
 * it will be set automatically by the device control
 * host library.
 */
#define APP_CONTROL_CMD_AP_VERSION 0x00
#define APP_CONTROL_CMD_AP_MIC_FROM_USB 0x01

#ifndef CMDSPEC_ALLOC_STRINGS
#define CMDSPEC_ALLOC_STRINGS 1
#endif

static cmd_t commands[] = {
        {APP_CONTROL_RESID_AP, "version", TYPE_UINT32, 0, APP_CONTROL_CMD_AP_VERSION, CMD_RO, 1, "Returns the Avona audio pipeline version"},
        {APP_CONTROL_RESID_AP, "mic_from_usb", TYPE_UINT8, 0, APP_CONTROL_CMD_AP_MIC_FROM_USB, CMD_RW, 1, "Microphone audio is received from the USB host when true"},
        {APP_CONTROL_RESID_AP, "fixed_point_cmd", TYPE_INT32, 24, 0x7F, CMD_RW, 2, "This is an example fixed point command"},
};

static char *command_param_type_name(cmd_param_type_t type)
{
    char *tstr;

    switch (type) {
    case TYPE_UINT8:
        tstr = "uint8";
        break;

    case TYPE_INT8:
        tstr = "int8";
        break;

    case TYPE_UINT32:
        tstr = "uint32";
        break;

    case TYPE_INT32:
        tstr = "int32";
        break;

    case TYPE_UINT64:
        tstr = "uint64";
        break;

    case TYPE_INT64:
        tstr = "int64";
        break;

    default:
        tstr = "unknown";
        break;
    }

    return tstr;
}

void command_list_print(void)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(commands); i++) {
        /* print shit about each command. do a get and/or set version */
        cmd_t *cmd = &commands[i];

        printf("%s\n", cmd->cmd_name);
        printf("\tThis command is %s ", cmd->rw == CMD_WO ? "write only" : cmd->rw == CMD_RO ? "read only" : "read/write");


        if (cmd->fractional_bits == 0) {
            printf("and has %d integer values of type %s\n", cmd->num_values, command_param_type_name(cmd->type));
        } else {
            printf("and has %d fixed point values of type %s with %d fractional bits\n", cmd->num_values, command_param_type_name(cmd->type), cmd->fractional_bits);
        }
        printf("\t%s\n", cmd->info);
    }
    printf("\n");
}

cmd_t *command_lookup(const char *s)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(commands); i++) {
        cmd_t *cmd = &commands[i];
        if (!strcmp(s, cmd->cmd_name)) {
            return cmd;
        }
    }

    return NULL;
}

static control_ret_t command_xfer(cmd_dir_t dir, cmd_t *cmd, void *data, size_t data_len)
{
    control_ret_t ret;

    if (dir == CMD_GET && cmd->rw != CMD_WO) {

        ret = control_read_command(cmd->resid,
                                   cmd->cmd,
                                   data,
                                   data_len);

    } else if (dir == CMD_SET && cmd->rw != CMD_RO) {

        ret = control_write_command(cmd->resid,
                                   cmd->cmd,
                                   data,
                                   data_len);

    } else {
        printf("%s is %s only\n", cmd->cmd_name, cmd->rw == CMD_WO ? "write" : "read");
        ret = CONTROL_BAD_COMMAND;
    }

    return ret;
}

#define SCALE_Q(SHIFT) (uint64_t) (1 << SHIFT)
static cmd_param_t command_bytes_to_value(cmd_t *cmd, void *data, int index)
{
    cmd_param_t value;
    float f;

    switch (cmd->type) {
    case TYPE_UINT8:
        value.u8 = ((uint8_t *)data)[index];
        f = value.u8;
        break;

    case TYPE_INT8:
        value.s8 = ((int8_t *)data)[index];
        f = value.s8;
        break;

    case TYPE_UINT32:
        value.u32 = ((uint32_t *)data)[index];
        f = value.u32;
        break;

    case TYPE_INT32:
        value.s32 = ((int32_t *)data)[index];
        f = value.s32;
        break;

    case TYPE_UINT64:
        value.u64 = ((uint64_t *)data)[index];
        f = value.u64;
        break;

    case TYPE_INT64:
        value.s64 = ((int64_t *)data)[index];
        f = value.s64;
        break;

    default:
        value.u64 = 0;
        f = 0;
        break;
    }

    if (cmd->fractional_bits > 0) {
        value.f = f / SCALE_Q(cmd->fractional_bits);
    }

    return value;
}

static void command_bytes_from_value(cmd_t *cmd, void *data, int index, cmd_param_t value)
{
    float f = value.f * SCALE_Q(cmd->fractional_bits);

    switch (cmd->type) {
    case TYPE_UINT8:
        value.u8 = cmd->fractional_bits > 0 ? f : value.u8;
        ((uint8_t *)data)[index] = value.u8;
        break;

    case TYPE_INT8:
        value.s8 = cmd->fractional_bits > 0 ? f : value.s8;
        ((int8_t *)data)[index] = value.s8;
        break;

    case TYPE_UINT32:
        value.u32 = cmd->fractional_bits > 0 ? f : value.u32;
        ((uint32_t *)data)[index] = value.u32;
        break;

    case TYPE_INT32:
        value.s32 = cmd->fractional_bits > 0 ? f : value.s32;
        ((int32_t *)data)[index] = value.s32;
        break;

    case TYPE_UINT64:
        value.u64 = cmd->fractional_bits > 0 ? f : value.u64;
        ((uint64_t *)data)[index] = value.u64;
        break;

    case TYPE_INT64:
        value.s64 = cmd->fractional_bits > 0 ? f : value.s64;
        ((int64_t *)data)[index] = value.s64;
        break;
    }
}

#define CLAMP(min, v, max) ((v) < (min) ? (min) : ((v) > (max) ? (max) : (v)))
cmd_param_t command_arg_string_to_value(cmd_t *cmd, const char *str)
{
    cmd_param_t value;

    if (cmd->fractional_bits > 0) {
        value.f = strtof(str, NULL);
    } else {
        int64_t tmp_val = strtoll(str, NULL, 0);

        switch (cmd->type) {
        case TYPE_UINT8:
            value.u8 = CLAMP(0, tmp_val, UINT8_MAX);
            break;

        case TYPE_INT8:
            value.s8 = CLAMP(INT8_MIN, tmp_val, INT8_MAX);
            break;

        case TYPE_UINT32:
            value.u32 = CLAMP(0, tmp_val, UINT32_MAX);
            break;

        case TYPE_INT32:
            value.s32 = CLAMP(INT32_MIN, tmp_val, INT32_MAX);
            break;

        case TYPE_UINT64:
            value.u64 = strtoull(str, NULL, 0);
            break;

        case TYPE_INT64:
            value.s64 = tmp_val;
            break;

        default:
            value.u64 = 0;
            break;
        }
    }

    return value;
}

void command_value_print(cmd_t *cmd, cmd_param_t arg)
{
    if (cmd->fractional_bits > 0) {
        printf("%f ", arg.f);
    } else {
        switch (cmd->type) {
        case TYPE_UINT8:
            printf("%" PRIu8 " ", arg.u8);
            break;

        case TYPE_INT8:
            printf("%" PRId8 " ", arg.s8);
            break;

        case TYPE_UINT32:
            printf("%" PRIu32 " ", arg.u32);
            break;

        case TYPE_INT32:
            printf("%" PRId32 " ", arg.s32);
            break;

        case TYPE_UINT64:
            printf("%" PRIu64 " ", arg.u64);
            break;

        case TYPE_INT64:
            printf("%" PRId64 " ", arg.s64);
            break;

        default:
            printf("0 ");
            break;
        }
    }
}

control_ret_t command_get(cmd_t *cmd, cmd_param_t values[], int num_values)
{
    control_ret_t ret = CONTROL_SUCCESS;

    if (num_values != cmd->num_values) {
        ret = CONTROL_DATA_LENGTH_ERROR;
    }

    if (ret == CONTROL_SUCCESS) {

        size_t data_len = cmd_param_type_size[cmd->type] * cmd->num_values;
        void *data = malloc(data_len);

        ret = command_xfer(CMD_GET, cmd, data, data_len);

        if (ret == CONTROL_SUCCESS) {
            //*values = calloc(cmd->num_values, sizeof(cmd_param_t));

            for (int i = 0; i < cmd->num_values; i++) {
                values[i] = command_bytes_to_value(cmd, data, i);
            }
        }

        free(data);
    }

    return ret;
}

control_ret_t command_set(cmd_t *cmd, const cmd_param_t values[], int num_values)
{
    control_ret_t ret = CONTROL_SUCCESS;

    if (num_values != cmd->num_values) {
        ret = CONTROL_DATA_LENGTH_ERROR;
    }

    if (ret == CONTROL_SUCCESS) {

        size_t data_len = cmd_param_type_size[cmd->type] * cmd->num_values;
        void *data = malloc(data_len);

        for (int i = 0; i < cmd->num_values; i++) {
            command_bytes_from_value(cmd, data, i, values[i]);
        }

        ret = command_xfer(CMD_SET, cmd, data, data_len);
        free(data);
    }

    return ret;
}



static char *cmd_strdup(char *str, size_t max_len)
{
#if CMDSPEC_ALLOC_STRINGS
    char *newstr = calloc(strnlen(str, max_len) + 1, sizeof(char));
    return strncat(newstr, str, max_len);
#else
    (void) max_len;
    return str;
#endif
}

cmd_t cmdspec_create(control_resid_t resid,
                     char *par_name,
                     cmd_param_type_t type,
                     unsigned offset,
                     cmd_rw_t rw,
                     unsigned num_values,
                     char *info)
{
    cmd_t cmd;
    cmd.resid = resid;
    cmd.cmd_name = cmd_strdup(par_name, CMDSPEC_PAR_NAME_MAX_LEN);
    cmd.type = type;
    cmd.cmd = offset;
    cmd.num_values = num_values;

    cmd.rw = rw;
    cmd.info = cmd_strdup(info, CMDSPEC_PAR_INFO_MAX_LEN);

    /* TODO: how about just calculating these when we need them rather than
     * here and storing them in the struct? */
//    cmd.device_rw_size = cmd_param_type_size[type];
//    cmd.app_read_result_size = num_values * get_app_read_result_size(type);

    return cmd;
}
