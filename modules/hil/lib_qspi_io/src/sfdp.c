// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT SFDP

#include "sfdp.h"
#include "xcore_utils.h"

size_t sfdp_flash_size_kbytes(sfdp_info_t *sfdp_info)
{
    size_t flash_size_kbytes;
    if (sfdp_info->basic_parameter_table.memory_density_is_exponent) {
        if (sfdp_info->basic_parameter_table.memory_density >=32 && sfdp_info->basic_parameter_table.memory_density <= 44) {
            flash_size_kbytes = 1 << (sfdp_info->basic_parameter_table.memory_density - 13);
        } else {
            flash_size_kbytes = 0;
        }
    } else {
        flash_size_kbytes = (1 + sfdp_info->basic_parameter_table.memory_density) >> 13;
    }

    return flash_size_kbytes;
}

size_t sfdp_flash_page_size_bytes(sfdp_info_t *sfdp_info)
{
    return 1 << sfdp_info->basic_parameter_table.page_size;
}

int sfdp_busy_poll_method(sfdp_info_t *sfdp_info,
                          uint8_t *instruction,
                          uint8_t *bit,
                          uint8_t *ready_value)
{
    uint32_t method_set = sfdp_info->basic_parameter_table.busy_poll_methods;
    if (method_set & SFDP_BUSY_POLL_ALT1_BM) {
        *instruction = 0x70;
        *bit = 7;
        *ready_value = 1;
    } else if (method_set & SFDP_BUSY_POLL_LEGACY_BM) {
        *instruction = 0x05;
        *bit = 0;
        *ready_value = 0;
    } else {
        return -1;
    }

    return 0;
}

int sfdp_quad_enable_method(sfdp_info_t *sfdp_info,
                            uint8_t *qe_reg,
                            uint8_t *qe_bit,
                            uint8_t *sr2_read_instruction,
                            uint8_t *sr2_write_instruction)
{
    switch (sfdp_info->basic_parameter_table.quad_enable_method) {
    case 0:
        *qe_reg = 0;
        *qe_bit = 0;
        *sr2_read_instruction = 0;
        *sr2_write_instruction = 0;
        break;
    case 1:
        *qe_reg = 2;
        *qe_bit = 1;
        *sr2_read_instruction = 0;
        *sr2_write_instruction = 0;
        break;
    case 2:
        *qe_reg = 1;
        *qe_bit = 6;
        *sr2_read_instruction = 0;
        *sr2_write_instruction = 0;
        break;
    case 3:
        *qe_reg = 2;
        *qe_bit = 7;
        *sr2_read_instruction = 0x3F;
        *sr2_write_instruction = 0x3E;
        break;
    case 4:
        *qe_reg = 2;
        *qe_bit = 1;
        *sr2_read_instruction = 0;
        *sr2_write_instruction = 0;
        break;
    case 5:
        *qe_reg = 2;
        *qe_bit = 1;
        *sr2_read_instruction = 0x35;
        *sr2_write_instruction = 0;
        break;
    case 6:
        *qe_reg = 2;
        *qe_bit = 1;
        *sr2_read_instruction = 0x35;
        *sr2_write_instruction = 0x31;
        break;
    default:
        return -1;
    }

    return 0;
}

static void sfdp_erase_table_sort(sfdp_info_t *sfdp_info)
{
    sfdp_parameter_table_t *t = &sfdp_info->basic_parameter_table;
    const int n = 4;
    int i, j;
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - i - 1; j++) {
            /* Treat size 0 as a maximum value to keep them at the end of the table */
            if (t->erase_info[j].size == 0 || (t->erase_info[j + 1].size != 0 && t->erase_info[j].size > t->erase_info[j + 1].size)) {
                uint8_t size = t->erase_info[j].size;
                uint8_t cmd = t->erase_info[j].cmd;
                t->erase_info[j].size = t->erase_info[j + 1].size;
                t->erase_info[j].cmd = t->erase_info[j + 1].cmd;
                t->erase_info[j + 1].size = size;
                t->erase_info[j + 1].cmd = cmd;
            }
        }
    }
}

bool sfdp_discover(sfdp_info_t *sfdp_info,
                  void *serial_flash_ctx,
                  SFDP_READ_CALLBACK_ATTR sfdp_read_cb_t sfdp_read)
{
    const uint32_t sfdp_signature = 0x50444653;
    const uint8_t  req_major_revision = 1;
    const uint8_t  req_min_minor_revision = 5;
    size_t table_read_length;

    sfdp_read(serial_flash_ctx, sfdp_info, 0x000000, sizeof(sfdp_header_t) + sizeof(sfdp_parameter_header_t));

    table_read_length = sizeof(uint32_t) * sfdp_info->basic_parameter_header.length;

    if (sfdp_info->sfdp_header.signature == sfdp_signature &&
            sfdp_info->sfdp_header.major_revision == req_major_revision &&
            sfdp_info->basic_parameter_header.major_revision == req_major_revision &&
            sfdp_info->sfdp_header.minor_revision >= req_min_minor_revision &&
            sfdp_info->basic_parameter_header.minor_revision >= req_min_minor_revision &&
            table_read_length >= sizeof(sfdp_parameter_table_t)) {

        debug_printf("Supported SFDP flash device found\n");

    } else {
        if (sfdp_info->sfdp_header.signature != sfdp_signature) {
            debug_printf("No SFDP flash device found\n");
        } else {
            debug_printf("Unsupported SFDP flash device found\n");
        }

        return false;
    }

    if (sizeof(sfdp_parameter_table_t) < table_read_length) {
        table_read_length = sizeof(sfdp_parameter_table_t);
    }

    sfdp_read(serial_flash_ctx, &sfdp_info->basic_parameter_table, sfdp_info->basic_parameter_header.table_address, table_read_length);

    /* Ensure that the erase table is sorted by size, with unused entries at the end. */
    sfdp_erase_table_sort(sfdp_info);

    return true;
}
