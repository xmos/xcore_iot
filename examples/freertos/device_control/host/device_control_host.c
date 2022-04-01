// Copyright 2020-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "argtable/argtable3.h"
#include "commands.h"

struct arg_lit *help, *test_cmd;
struct arg_str *get, *set, *cmd_args;
struct arg_end *end;

int main(int argc, char **argv)
{
    control_ret_t ret = CONTROL_ERROR;

    void *argtable[] = {
        help    = arg_lit0(NULL, "help", "display this help and exit"),
        test_cmd = arg_lit0(NULL, "test_cmd", "display test value and exit"),

        get     = arg_str0("g", "get", "<cmd>", "Sends the specified get command and prints the return value(s). Must not be used with --set."),
        set     = arg_str0("s", "set", "<cmd>", "Sends the specified set command with the provided argument(s). Must not be used with --get."),

        cmd_args  = arg_strn(NULL, NULL, "<arg>", 0, 100,  "Command argument values for use with set"),

        end     = arg_end(20),
    };


    int nerrors;
    nerrors = arg_parse(argc, argv, argtable);

    if (help->count > 0) {
        printf("Usage: %s", argv[0]);
        arg_print_syntax(stdout, argtable, "\n");
        printf("Control tool for xcore_sdk\n\n");
        arg_print_glossary_gnu(stdout, argtable);
        printf("The following commands are supported:\n");
        command_list_print();
        return 0;
    }

    if (nerrors > 0) {
        /* Display the error details contained in the arg_end struct.*/
        arg_print_errors(stdout, end, argv[0]);
        printf("Try '%s --help' for more information.\n", argv[0]);
        return 1;
    }

#if USE_USB
    ret = control_init_usb(0x20B1, 0x1010, 0);
#elif USE_I2C
    ret = control_init_i2c(0x42);
#endif

    if (ret == CONTROL_SUCCESS) {
        cmd_t *cmd;
        cmd_param_t *cmd_values = NULL;

        ret = CONTROL_ERROR;

        if (get->count != 0 && set->count == 0) {

            if (cmd_args->count == 0) {

                cmd = command_lookup(get->sval[0]);
                if (cmd != NULL) {
                    cmd_values = calloc(cmd->num_values, sizeof(cmd_param_t));
                    ret = command_get(cmd, cmd_values, cmd->num_values);
                    printf("Command %s sent with resid %i\n", get->sval[0], cmd->resid);
                } else {
                    printf("Command %s not recognized\n", get->sval[0]);
                }
            } else {
                printf("Get commands do not take any arguments\n");
            }

            if (cmd_values != NULL) {
                printf("Bytes received are:\n");
                for (int i = 0; i < cmd->num_values; i++) {
                    // print the actual value
                    command_value_print(cmd, cmd_values[i]);
                }
                printf("\n");
            }

        } else if (set->count != 0 && get->count == 0) {

            cmd = command_lookup(set->sval[0]);
            if (cmd != NULL) {

                if (cmd->num_values == cmd_args->count) {
                    cmd_values = calloc(cmd_args->count, sizeof(cmd_param_t));

                    for (int i = 0; i < cmd_args->count; i++) {
                        cmd_values[i] = command_arg_string_to_value(cmd, cmd_args->sval[i]);
                    }

                    ret = command_set(cmd, cmd_values, cmd_args->count);
                } else {
                    printf("The command %s requires %d argument%s\n", cmd->cmd_name, cmd->num_values, cmd->num_values == 1 ? "" : "s");
                }

            } else {
                printf("Command %s not recognized\n", get->sval[0]);
            }

        } else if (set->count != 0 && get->count != 0) {
            printf("Must not specify both --get and --set commands\n");
        }

        if (cmd_values != NULL) {
            free(cmd_values);
        }
    }

    return ret == CONTROL_SUCCESS ? 0 : 1;
}
