/*
 * commands.h
 *
 *  Created on: Aug 25, 2021
 *      Author: mbruno
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

#include <stdint.h>
#include "device_control_host.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define CMDSPEC_PAR_NAME_MAX_LEN (40)
#define CMDSPEC_PAR_INFO_MAX_LEN (200)
#define CMD_MAX_BYTES            (64)

typedef enum {CMD_RO, CMD_WO, CMD_RW} cmd_rw_t;
typedef enum {CMD_GET, CMD_SET} cmd_dir_t;

typedef union {
    uint8_t u8;
    int8_t s8;
    uint32_t u32;
    int32_t s32;
    uint64_t u64;
    int64_t s64;
    float f;
} cmd_param_t;

/*
 * TODO: Maybe number of fractional bits can be stored in the
 * cmd_t struct. Then we'd only need type size (1,4,8) and
 * signed/unsigned. 0 fractional bits for ints.
 */
typedef enum {
//    TYPE_FIXED_0_32,
//    TYPE_FIXED_1_31,
//    TYPE_FIXED_7_24,
//    TYPE_FIXED_8_24,
//    TYPE_FIXED_16_16,
    TYPE_UINT8,
    TYPE_INT8,
    TYPE_UINT32,
    TYPE_INT32,
    TYPE_UINT64,
    TYPE_INT64,
} cmd_param_type_t;

static const size_t cmd_param_type_size[] = {
        [TYPE_UINT8] = sizeof(uint8_t),
        [TYPE_INT8] = sizeof(int8_t),
        [TYPE_UINT32] = sizeof(uint32_t),
        [TYPE_INT32] = sizeof(int32_t),
        [TYPE_UINT64] = sizeof(uint64_t),
        [TYPE_INT64] = sizeof(int64_t),
};

typedef struct {
  control_resid_t resid;
  char *cmd_name;
  cmd_param_type_t type;
  int fractional_bits;
  control_cmd_t cmd;
  cmd_rw_t rw;
  unsigned num_values; //no. of values read or written from the device
  char *info;
//  unsigned device_rw_size; //no. of bytes per value read or written from the device for a given type.
//  unsigned app_read_result_size; //for read commands, the total amount of memory app needs to allocate and send for returning read values into .
} cmd_t;

void command_list_print(void);
cmd_t *command_lookup(const char *s);
cmd_param_t command_arg_string_to_value(cmd_t *cmd, const char *str);
void command_value_print(cmd_t *cmd, cmd_param_t arg);
control_ret_t command_get(cmd_t *cmd, cmd_param_t values[], int num_values);
control_ret_t command_set(cmd_t *cmd, const cmd_param_t values[], int num_values);


#endif /* COMMANDS_H_ */
