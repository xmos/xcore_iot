// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef SFDP_H_
#define SFDP_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define SFDP_READ_INSTRUCTION 0x5A

typedef struct {
    uint32_t signature;
    uint8_t minor_revision;
    uint8_t major_revision;
    uint8_t nph;
    uint8_t : 8;
} sfdp_header_t;

typedef struct {
    uint8_t id_lsb;
    uint8_t minor_revision;
    uint8_t major_revision;
    uint8_t length;
    uint32_t table_address : 24;
    uint8_t id_msb;
} sfdp_parameter_header_t;

enum {
    sfdp_3_byte_address      = 0,
    sfdp_3_or_4_byte_address = 1,
    sfdp_4_byte_address      = 2,
};

#define SFDP_BUSY_POLL_LEGACY_BM 0x01
#define SFDP_BUSY_POLL_ALT1_BM   0x02

typedef struct {
    /* 1st DWORD */
    uint32_t : 2; /* legacy */
    uint32_t : 1; /* legacy */
    uint32_t : 1; /* legacy */
    uint32_t : 1; /* legacy */
    uint32_t : 3; /* unused */
    uint32_t : 8; /* erase table used instead */
    uint32_t : 1; /* supports 1-1-2 fast read */
    uint32_t address_bytes : 2;
    uint32_t : 1; /* DTR clocking */
    uint32_t : 1; /* supports 1-2-2 fast read */
    uint32_t supports_144_fast_read: 1;
    uint32_t supports_114_fast_read: 1;
    uint32_t : 9; /* unused */

    /* 2nd DWORD */
    uint32_t memory_density : 30;
    uint32_t memory_density_is_exponent : 1;

    /* 3rd DWORD */
    uint8_t quad_144_read_dummy_clocks : 5;
    uint8_t quad_144_read_mode_clocks : 3;
    uint8_t quad_144_read_cmd;
    uint8_t quad_114_read_dummy_clocks : 5;
    uint8_t quad_114_read_mode_clocks : 3;
    uint8_t quad_114_read_cmd;

    /* 4th DWORD */
    uint32_t : 32;
    /* 5th DWORD */
    uint32_t : 32;
    /* 6th DWORD */
    uint32_t : 32;
    /* 7th DWORD */
    uint32_t : 32;

    /* 8th and 9th DWORD */
    struct {
        uint8_t size;
        uint8_t cmd;
    } erase_info[4];

    /* 10th DWORD */
    uint32_t : 32; /* typical and maximum erase times */
                   /* TODO could be nice for timeouts */

    /* 11th DWORD */
    /* typical and max chip erase and program times. page size. */
    uint32_t typ_to_max_prog_time_multiplier : 4;
    uint32_t page_size : 4;
    uint32_t page_prog_time_typ : 6;
    uint32_t byte_prog_time_first_typ : 5;
    uint32_t byte_prog_time_addl_typ : 5;
    uint32_t chip_erase_time_typ : 7;
    uint32_t : 1; /* reserved */

    /* 12th DWORD */
    uint32_t : 32; /* suspend/resume info */

    /* 13th DWORD */
    uint32_t : 32; /* suspend/resume instructions */

    /* 14th DWORD */
    uint32_t : 2; /* reserved */
    uint32_t busy_poll_methods : 6;
    uint32_t  : 7; /* powerdown info */
    uint32_t  : 8; /* powerdown info */
    uint32_t  : 8; /* powerdown info */
    uint32_t  : 1; /* powerdown info */

    /* 15th DWORD */
    uint32_t : 4; /* 4-4-4 mode disable */
    uint32_t : 5; /* 4-4-4 mode enable */
    uint32_t xip_mode_supported : 1;
    uint32_t xip_mode_exit_method : 6;
    uint32_t xip_mode_entry_method : 4;
    uint32_t quad_enable_method : 3;
    uint32_t hold_reset_disable : 1;
    uint32_t : 8; /* reserved */

    /* 16th DWORD */
    uint32_t status_reg_1_info : 7;
    uint32_t : 1; /* reserved */
    uint32_t soft_reset_sequence : 6;
    uint32_t four_byte_address_exit_method : 10;
    uint32_t four_byte_address_enter_method : 8;

} sfdp_parameter_table_t;

typedef struct {
    sfdp_header_t sfdp_header;
    sfdp_parameter_header_t basic_parameter_header;
    sfdp_parameter_table_t basic_parameter_table;
} sfdp_info_t;

#define SFDP_READ_CALLBACK_ATTR __attribute__((fptrgroup("sfdp_read_cb_fptr_grp")))

typedef void (*sfdp_read_cb_t)(void *flash_ctx, void *data, uint32_t address, size_t len);

size_t sfdp_flash_size_kbytes(sfdp_info_t *sfdp_info);
size_t sfdp_flash_page_size_bytes(sfdp_info_t *sfdp_info);
int sfdp_busy_poll_method(sfdp_info_t *sfdp_info,
                          uint8_t *instruction,
                          uint8_t *bit,
                          uint8_t *ready_value);
int sfdp_quad_enable_method(sfdp_info_t *sfdp_info,
                            uint8_t *qe_reg,
                            uint8_t *qe_bit,
                            uint8_t *sr2_read_instruction,
                            uint8_t *sr2_write_instruction);

bool sfdp_discover(sfdp_info_t *sfdp_info,
                   void *serial_flash_ctx,
                   sfdp_read_cb_t sfdp_read);

#endif /* SFDP_H_ */
