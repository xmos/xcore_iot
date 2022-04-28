// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT QSPI_FLASH

#include "qspi_flash.h"
#include "sfdp.h"
#if QSPI_FLASH_SANITY_CHECKS
	/*
	 * Ensure NDEBUG is not defined when the
	 * flash sanity checks are enabled.
	 */
	#if defined(NDEBUG)
		#undef NDEBUG
	#endif
#endif
#include "xcore_utils.h"
#include <xcore/assert.h>
#include <xcore/hwtimer.h>

/*
 * TODO: This isn't fully supported yet
 */
#define FOUR_BYTE_ADDRESS_SUPPORT 0

#define BARRIER() asm volatile("": : :"memory")

#define WRITE_ENABLE_COMMAND      QSPI_IO_BYTE_TO_MOSI(0x06)
#define WRITE_DISABLE_COMMAND     QSPI_IO_BYTE_TO_MOSI(0x04)

#define ERASE_CHIP_COMMAND        QSPI_IO_BYTE_TO_MOSI(0xC7)

#define PP_1_1_1_COMMAND          QSPI_IO_BYTE_TO_MOSI(0x02)
#define PP_1_1_4_COMMAND          QSPI_IO_BYTE_TO_MOSI(0x32)
#define PP_1_4_4_COMMAND          QSPI_IO_BYTE_TO_MOSI(0x38)

#define READ_STATUS_REG_COMMAND   QSPI_IO_BYTE_TO_MOSI(0x05)
#define READ_ID_COMMAND           QSPI_IO_BYTE_TO_MOSI(0x9F)

#define WRITE_STATUS_REG_COMMAND  QSPI_IO_BYTE_TO_MOSI(0x01)

#define FAST_READ_COMMAND         QSPI_IO_BYTE_TO_MOSI(0x0B)
#define QUAD_IO_READ_CMD_VAL      0xEB
#define QUAD_IO_READ_COMMAND      QSPI_IO_BYTE_TO_MOSI(QUAD_IO_READ_CMD_VAL)

#if FOUR_BYTE_ADDRESS_SUPPORT
#define QUAD_IO_READ_DUMMY_CYCLES 6
#else
#define QUAD_IO_READ_DUMMY_CYCLES 4
#endif
#define FAST_READ_DUMMY_CYCLES    8

bool qspi_flash_quad_enable_write(qspi_flash_ctx_t *ctx, bool set)
{
    uint8_t status[2];
    uint8_t quad_enable_bitmask;
    const uint32_t no_cmd = QSPI_IO_BYTE_TO_MOSI(0x00);

    if (ctx->qe_reg == 0) {
        return true;
    }

    xassert(ctx->qe_reg == 1 || ctx->qe_reg == 2);
    xassert(ctx->qe_bit >= 0 && ctx->qe_bit <= 7);

    quad_enable_bitmask = 1 << ctx->qe_bit;

    if (ctx->qe_reg == 1 || ctx->sr2_read_cmd == no_cmd) {
        qspi_flash_read_status_register(ctx, status, ctx->qe_reg);
    } else {
        qspi_flash_read_status_register(ctx, &status[0], 1);
        qspi_flash_read_register(ctx, ctx->sr2_read_cmd, &status[1], 1);
    }

    if (!!(status[ctx->qe_reg - 1] & quad_enable_bitmask) != set) {
        if (set) {
            status[ctx->qe_reg - 1] |= quad_enable_bitmask;
        } else {
            status[ctx->qe_reg - 1] &= ~quad_enable_bitmask;
        }

        qspi_flash_write_enable(ctx);

        if (ctx->qe_reg == 1 || ctx->sr2_write_cmd == no_cmd) {
            qspi_flash_write_status_register(ctx, status, ctx->qe_reg);
        } else {
            qspi_flash_write_register(ctx, ctx->sr2_write_cmd, &status[1], 1);
        }

        qspi_flash_wait_while_write_in_progress(ctx);

        status[0] = 0;
        status[1] = 0;

        if (ctx->qe_reg == 1 || ctx->sr2_read_cmd == no_cmd) {
            qspi_flash_read_status_register(ctx, status, ctx->qe_reg);
        } else {
            qspi_flash_read_register(ctx, ctx->sr2_read_cmd, &status[1], 1);
        }

        return !!(status[ctx->qe_reg - 1] & quad_enable_bitmask) == set;
    } else {
        return true;
    }
}

void qspi_flash_write_enable(qspi_flash_ctx_t *ctx)
{
	qspi_io_ctx_t *qspi_io_ctx = &ctx->qspi_io_ctx;
	uint8_t status;

	do {
	    qspi_io_start_transaction(qspi_io_ctx, WRITE_ENABLE_COMMAND, 8, qspi_io_full_speed);
	    qspi_io_end_transaction(qspi_io_ctx);
	    qspi_flash_read_status_register(ctx, &status, 1);
	} while ((status & QSPI_FLASH_STATUS_REG_WEL_BM) == 0);
}

void qspi_flash_write_disable(qspi_flash_ctx_t *ctx)
{
	qspi_io_ctx_t *qspi_io_ctx = &ctx->qspi_io_ctx;
    uint8_t status;

    do {
        qspi_io_start_transaction(qspi_io_ctx, WRITE_DISABLE_COMMAND, 8, qspi_io_full_speed);
        qspi_io_end_transaction(qspi_io_ctx);
        qspi_flash_read_status_register(ctx, &status, 1);
    } while ((status & QSPI_FLASH_STATUS_REG_WEL_BM) != 0);
}

bool qspi_flash_write_in_progress(qspi_flash_ctx_t *ctx)
{
	uint8_t status_reg;
	qspi_flash_read_register(ctx, ctx->busy_poll_cmd, &status_reg, 1);

	return ((status_reg >> ctx->busy_poll_bit) & 1) != ctx->busy_poll_ready_value;
}

void qspi_flash_wait_while_write_in_progress(qspi_flash_ctx_t *ctx)
{
	qspi_flash_poll_register(ctx, ctx->busy_poll_cmd, (1 << ctx->busy_poll_bit), ctx->busy_poll_ready_value << ctx->busy_poll_bit);
}

void qspi_flash_erase(qspi_flash_ctx_t *ctx,
                      uint32_t address,
                      qspi_flash_erase_length_t erase_length)
{
	qspi_io_ctx_t *qspi_io_ctx = &ctx->qspi_io_ctx;

	size_t cycles;
	uint32_t cmd;
	uint8_t *address_bytes;

#if QSPI_FLASH_SANITY_CHECKS
	uint8_t status_reg;
	qspi_flash_read_status_register(ctx, &status_reg, 1);
	xassert(status_reg & QSPI_FLASH_STATUS_REG_WEL_BM);
#endif

	/*
	 * Manipulate the address so that in memory it is:
	 * {byte 2, byte 1, byte 0, XX}
	 * it will get sent out as 3 bytes.
	 */
	address = byterev(address << 8);
	address_bytes = (uint8_t *) &address;

	xassert(erase_length >= 0 && erase_length <= qspi_flash_erase_chip);
	if (erase_length > qspi_flash_erase_chip) {
	    return;
	}

	if (erase_length == qspi_flash_erase_chip) {
        cmd = ERASE_CHIP_COMMAND;
        cycles = 8;
	} else {
	    xassert(ctx->erase_info[erase_length].size_log2 > 0);
	    if (ctx->erase_info[erase_length].size_log2 <= 0) {
	        return;
	    }
	    cmd = ctx->erase_info[erase_length].cmd;
	    cycles = 32;
	}

	qspi_io_start_transaction(qspi_io_ctx, cmd, cycles, qspi_io_full_speed);
	if (cycles == 32) {
		qspi_io_mosi_out(qspi_io_ctx, qspi_io_transfer_normal, address_bytes, 3);
	}
	qspi_io_end_transaction(qspi_io_ctx);
}

void qspi_flash_write_register(qspi_flash_ctx_t *ctx,
                               uint32_t cmd,
                               const uint8_t *val,
                               size_t len)
{
	qspi_io_ctx_t *qspi_io_ctx = &ctx->qspi_io_ctx;

	size_t cycles = 8 +      /* 8 cycles for the command */
	                8 * len; /* 2 cycles per byte */

	qspi_io_start_transaction(qspi_io_ctx, cmd, cycles, qspi_io_full_speed);
	qspi_io_mosi_out(qspi_io_ctx, qspi_io_transfer_normal, val, len);
	qspi_io_end_transaction(qspi_io_ctx);
}

void qspi_flash_write_status_register(qspi_flash_ctx_t *ctx,
                                      const uint8_t *val,
                                      size_t len)
{
#if QSPI_FLASH_SANITY_CHECKS
	uint8_t status_reg;
	qspi_flash_read_status_register(ctx, &status_reg, 1);
	xassert(status_reg & QSPI_FLASH_STATUS_REG_WEL_BM);
#endif

	qspi_flash_write_register(ctx, WRITE_STATUS_REG_COMMAND, val, len);
}

void qspi_flash_read_register(qspi_flash_ctx_t *ctx,
                              uint32_t cmd,
                              uint8_t *val,
                              size_t len)
{
	qspi_io_ctx_t *qspi_io_ctx = &ctx->qspi_io_ctx;

	size_t cycles = 8 +      /* 8 cycles for the command */
	                8 * len; /* 8 cycles per byte */

	qspi_io_start_transaction(qspi_io_ctx, cmd, cycles, qspi_io_spi_read);
	qspi_io_sio_direction_input(qspi_io_ctx);
	qspi_io_miso_in(qspi_io_ctx, qspi_io_transfer_normal, val,
		8 + /* 8 cycles for the command */
		7,  /* input on the last cycle of the first byte */
		len);
	qspi_io_end_transaction(qspi_io_ctx);
}

void qspi_flash_read_status_register(qspi_flash_ctx_t *ctx,
                                     uint8_t *val,
                                     size_t len)
{
	qspi_flash_read_register(ctx, READ_STATUS_REG_COMMAND, val, len);
}

void qspi_flash_read_id(qspi_flash_ctx_t *ctx,
                        uint8_t *val,
                        size_t len)
{
	qspi_flash_read_register(ctx, READ_ID_COMMAND, val, len);
}

void qspi_flash_poll_register(qspi_flash_ctx_t *ctx,
                              uint32_t cmd,
                              const uint8_t mask,
                              const uint8_t val)
{
	qspi_io_ctx_t *qspi_io_ctx = &ctx->qspi_io_ctx;

	size_t cycles = 8 +     /* 8 cycles for the command */
	                8 * 2;  /* Read the register at least twice */

	qspi_io_start_transaction(qspi_io_ctx, cmd, cycles, qspi_io_spi_read);
	qspi_io_sio_direction_input(qspi_io_ctx);
	qspi_io_miso_poll(qspi_io_ctx, mask, val,
		8 + /* 8 cycles for the command */
		7);  /* input on the last cycle of the first byte */
	qspi_io_end_transaction(qspi_io_ctx);
}

void qspi_flash_poll_status_register(qspi_flash_ctx_t *ctx,
                                     const uint8_t mask,
                                     const uint8_t val)
{
	qspi_flash_poll_register(ctx, READ_STATUS_REG_COMMAND, mask, val);
}

__attribute__((always_inline))
static void qspi_flash_fast_read_i(qspi_flash_ctx_t *ctx,
                                   uint32_t cmd,
                                   uint8_t *data,
                                   uint32_t address,
                                   size_t len)
{
    qspi_io_ctx_t *qspi_io_ctx = &ctx->qspi_io_ctx;
    uint8_t *address_bytes;

    size_t cycles = 8 +                      /* 8 cycles for the command */
                    24 +                     /* 24 cycles for the address */
                    FAST_READ_DUMMY_CYCLES + /* dummy cycles */
                    8 * len;                 /* 8 cycles per byte */

    size_t input_cycle = 8 +                      /* 8 cycles for the command */
                         24 +                     /* 24 cycles for the address */
                         FAST_READ_DUMMY_CYCLES + /* dummy cycles */
                         7;                       /* input on the last cycle of the first byte */


    /*
     * Manipulate the address so that in memory it is:
     * {byte 2, byte 1, byte 0, XX}
     * it will get sent out as 3 bytes.
     */
    address = byterev(address << 8);
    address_bytes = (uint8_t *) &address;

    qspi_io_start_transaction(qspi_io_ctx, cmd, cycles, qspi_io_full_speed);
    qspi_io_mosi_out(qspi_io_ctx, qspi_io_transfer_normal, address_bytes, 3);
    qspi_io_sio_direction_input(qspi_io_ctx);
    qspi_io_miso_in(qspi_io_ctx, qspi_io_transfer_normal, data, input_cycle, len);
    qspi_io_end_transaction(qspi_io_ctx);
}

void qspi_flash_fast_read(qspi_flash_ctx_t *ctx,
                          uint8_t *data,
                          uint32_t address,
                          size_t len)
{
    qspi_flash_ctx_t local_ctx = *ctx;
    qspi_flash_fast_read_i(&local_ctx, QSPI_IO_BYTE_TO_MOSI(FAST_READ_COMMAND), data, address, len);
}

__attribute__ ((noinline))
static void qspi_flash_read_calc(const int xip,
#if FOUR_BYTE_ADDRESS_SUPPORT
                                 const int four_byte_address,
#endif
                                 uint32_t *address,
                                 size_t len,
                                 size_t *cycles,
                                 size_t *input_cycle)
{
	/* The first input should occur on either the fourth
	 * byte, or the last byte, whichever is first. */
	const size_t first_input_byte = len > 4 ? 4 : len;

#if FOUR_BYTE_ADDRESS_SUPPORT
    if (xip) {
        *cycles = (four_byte_address ? 8 : 6) + /* 6 or 8 cycles for address */
                 QUAD_IO_READ_DUMMY_CYCLES + /* dummy cycles */
                 2 * len; /* 2 cycles per byte */

        *input_cycle = (four_byte_address ? 8 : 6) + /* 6 or 8 cycles for address */
                      QUAD_IO_READ_DUMMY_CYCLES + /* dummy cycles */
                      2 * first_input_byte - 1; /* input on the last cycle of the first input byte */
    } else {
        *cycles = 8 + /* 8 cycles each for command */
                 (four_byte_address ? 8 : 6) + /* 6 or 8 cycles for address */
                 QUAD_IO_READ_DUMMY_CYCLES + /* dummy cycles */
                 2 * len; /* 2 cycles per byte */

        *input_cycle = 8 + /* 8 cycles each for command */
                      (four_byte_address ? 8 : 6) + /* 6 or 8 cycles for address */
                      QUAD_IO_READ_DUMMY_CYCLES + /* dummy cycles */
                      2 * first_input_byte - 1; /* input on the last cycle of the first input byte */
    }
#else
	if (xip) {
		*cycles = 8 + /* 8 cycles for address */
		         QUAD_IO_READ_DUMMY_CYCLES + /* dummy cycles */
		         2 * len; /* 2 cycles per byte */

		*input_cycle = 8 + /* 8 cycles for address */
		              QUAD_IO_READ_DUMMY_CYCLES + /* dummy cycles */
		              2 * first_input_byte - 1; /* input on the last cycle of the first input byte */
	} else {
		*cycles = 8 * 2 + /* 8 cycles each for command and address */
		         QUAD_IO_READ_DUMMY_CYCLES + /* dummy cycles */
		         2 * len; /* 2 cycles per byte */

		*input_cycle = 8 * 2 + /* 8 cycles each for command and address */
		              QUAD_IO_READ_DUMMY_CYCLES + /* dummy cycles */
		              2 * first_input_byte - 1; /* input on the last cycle of the first input byte */
	}
#endif

#if FOUR_BYTE_ADDRESS_SUPPORT
	if (!four_byte_address) {
#endif
        /*
         * The address is really contained in the upper 24 bits.
         * The lower 8 bits are essentially 2 dummy cycles.
         * Rotate, such that the MSB of address is sent out during
         * the first two dummy cycles. Some flashes use this to
         * enter "performance" or "XIP" mode.
         */
        *address = (*address << 8) | (*address >> 24);
#if FOUR_BYTE_ADDRESS_SUPPORT
	}
#endif
}

__attribute__((always_inline))
inline void qspi_flash_read_i(qspi_flash_ctx_t *ctx,
                              const qspi_io_transfer_mode_t transfer_mode,
                              const int xip,
#if FOUR_BYTE_ADDRESS_SUPPORT
                              const int four_byte_address,
#endif
                              uint8_t *data,
                              uint32_t address,
                              size_t len)
{
	qspi_io_ctx_t *qspi_io_ctx = &ctx->qspi_io_ctx;
	size_t cycles;
	size_t input_cycle;

	qspi_flash_read_calc(xip,
#if FOUR_BYTE_ADDRESS_SUPPORT
                         four_byte_address,
#endif
                         &address,
                         len,
                         &cycles,
                         &input_cycle);

	if (xip) {
		qspi_io_start_transaction(qspi_io_ctx, address, cycles, qspi_io_full_speed);
	} else {
		qspi_io_start_transaction(qspi_io_ctx, QUAD_IO_READ_COMMAND, cycles, qspi_io_full_speed);
		qspi_io_words_out(qspi_io_ctx, qspi_io_transfer_normal, &address, 1);
	}

#if FOUR_BYTE_ADDRESS_SUPPORT
	if (four_byte_address) {
	    uint8_t mode = 0xFF;
	    qspi_io_bytes_out(qspi_io_ctx, transfer_mode, &mode, 1);
	}
#endif

	qspi_io_sio_direction_input(qspi_io_ctx);
	qspi_io_bytes_in(qspi_io_ctx, transfer_mode, data, input_cycle, len);
	qspi_io_end_transaction(qspi_io_ctx);
}

void qspi_flash_read(qspi_flash_ctx_t *ctx,
                     uint8_t *data,
                     uint32_t address,
                     size_t len)
{
    qspi_flash_ctx_t local_ctx = *ctx;
	qspi_flash_read_i(&local_ctx, qspi_io_transfer_normal, 0,
#if FOUR_BYTE_ADDRESS_SUPPORT
	                  0,
#endif
	                  data, address, len);
}

void qspi_flash_read_nibble_swapped(qspi_flash_ctx_t *ctx,
                                    uint8_t *data,
                                    uint32_t address,
                                    size_t len)
{
    qspi_flash_ctx_t local_ctx = *ctx;
	qspi_flash_read_i(&local_ctx, qspi_io_transfer_nibble_swap, 0,
#if FOUR_BYTE_ADDRESS_SUPPORT
                      0,
#endif
	                  data, address, len);
}

void qspi_flash_xip_read(qspi_flash_ctx_t *ctx,
                         uint8_t *data,
                         uint32_t address,
                         size_t len)
{
    qspi_flash_ctx_t local_ctx = *ctx;
	qspi_flash_read_i(&local_ctx, qspi_io_transfer_normal, 1,
#if FOUR_BYTE_ADDRESS_SUPPORT
                      0,
#endif
	                  data, address, len);
}

void qspi_flash_xip_read_nibble_swapped(qspi_flash_ctx_t *ctx,
                         uint8_t *data,
                         uint32_t address,
                         size_t len)
{
    qspi_flash_ctx_t local_ctx = *ctx;
	qspi_flash_read_i(&local_ctx, qspi_io_transfer_nibble_swap, 1,
#if FOUR_BYTE_ADDRESS_SUPPORT
                      0,
#endif
	                  data, address, len);
}

__attribute__((always_inline))
inline void qspi_flash_write_i(qspi_flash_ctx_t *ctx,
                               const qspi_io_transfer_mode_t transfer_mode,
                               const uint8_t *data,
                               uint32_t address,
                               size_t len)
{
	qspi_io_ctx_t *qspi_io_ctx = &ctx->qspi_io_ctx;

	uint8_t *address_bytes;
	uint32_t pp_cmd;
	size_t cycles;

#if QSPI_FLASH_SANITY_CHECKS
	uint8_t status_reg;
	qspi_flash_read_status_register(ctx, &status_reg, 1);
	xassert(status_reg & QSPI_FLASH_STATUS_REG_WEL_BM);
#endif

	switch (ctx->quad_page_program_cmd) {
	case qspi_flash_page_program_1_1_4:
        pp_cmd = PP_1_1_4_COMMAND;
        cycles = 8  +  /* 8 cycles for command */
                 24 +  /* 24 cycles for address */
                 2 * len; /* 2 cycles per byte */
	    break;
	case qspi_flash_page_program_1_4_4:
        pp_cmd = PP_1_4_4_COMMAND;
        cycles = 8 +   /* 8 cycles for command */
                 6 +   /* 6 cycles for address */
                 2 * len; /* 2 cycles per byte */
	    break;
	case qspi_flash_page_program_1_1_1:
	default:
        pp_cmd = PP_1_1_1_COMMAND;
        cycles = 8  +  /* 8 cycles for command */
                 24 +  /* 24 cycles for address */
                 8 * len; /* 8 cycles per byte */
        break;
	}

	/*
	 * Manipulate the address so that in memory it is:
	 * {byte 2, byte 1, byte 0, XX}
	 * it will get sent out as 3 bytes.
	 */
	address = byterev(address << 8);
	address_bytes = (uint8_t *) &address;

	qspi_io_start_transaction(qspi_io_ctx, pp_cmd, cycles, qspi_io_full_speed);
    switch (ctx->quad_page_program_cmd) {
    case qspi_flash_page_program_1_1_4:
        qspi_io_mosi_out(qspi_io_ctx, qspi_io_transfer_normal, address_bytes, 3);
        qspi_io_bytes_out(qspi_io_ctx, transfer_mode, data, len);
        break;
    case qspi_flash_page_program_1_4_4:
        qspi_io_bytes_out(qspi_io_ctx, qspi_io_transfer_normal, address_bytes, 3);
        qspi_io_bytes_out(qspi_io_ctx, transfer_mode, data, len);
        break;
    case qspi_flash_page_program_1_1_1:
    default:
        qspi_io_mosi_out(qspi_io_ctx, qspi_io_transfer_normal, address_bytes, 3);
        qspi_io_mosi_out(qspi_io_ctx, transfer_mode, data, len);
        break;
    }

	qspi_io_end_transaction(qspi_io_ctx);
}

void qspi_flash_write(qspi_flash_ctx_t *ctx,
                      const uint8_t *data,
                      uint32_t address,
                      size_t len)
{
    qspi_flash_ctx_t local_ctx = *ctx;
	qspi_flash_write_i(&local_ctx, qspi_io_transfer_normal, data, address, len);
}

void qspi_flash_write_nibble_swapped(qspi_flash_ctx_t *ctx,
                      const uint8_t *data,
                      uint32_t address,
                      size_t len)
{
    qspi_flash_ctx_t local_ctx = *ctx;
	qspi_flash_write_i(&local_ctx, qspi_io_transfer_nibble_swap, data, address, len);
}

SFDP_READ_CALLBACK_ATTR
void qspi_flash_sfdp_read(qspi_flash_ctx_t *ctx,
                          uint8_t *data,
                          uint32_t address,
                          size_t len)
{
    qspi_flash_ctx_t local_ctx = *ctx;
    qspi_flash_fast_read_i(&local_ctx, QSPI_IO_BYTE_TO_MOSI(SFDP_READ_INSTRUCTION), data, address, len);
}

void qspi_flash_deinit(qspi_flash_ctx_t *ctx)
{
	qspi_io_ctx_t *qspi_io_ctx = &ctx->qspi_io_ctx;

	qspi_io_deinit(qspi_io_ctx);
}

void qspi_flash_init(qspi_flash_ctx_t *ctx)
{
    sfdp_info_t sfdp_info;
	qspi_io_ctx_t *qspi_io_ctx = &ctx->qspi_io_ctx;

	if (!ctx->custom_clock_setup) {
		ctx->source_clock = qspi_io_source_clock_ref;

		/* 100 / (2 * 2) = 25 MHz */
		qspi_io_ctx->full_speed_clk_divisor       = 2;
		qspi_io_ctx->full_speed_sclk_sample_delay = 0;
		qspi_io_ctx->full_speed_sclk_sample_edge  = qspi_io_sample_edge_falling;
		qspi_io_ctx->full_speed_sio_pad_delay     = 0;

		/* 100 / (2 * 2) = 25 MHz */
		qspi_io_ctx->spi_read_clk_divisor       = 2;
		qspi_io_ctx->spi_read_sclk_sample_delay = 0;
		qspi_io_ctx->spi_read_sclk_sample_edge  = qspi_io_sample_edge_falling;
		qspi_io_ctx->spi_read_sio_pad_delay     = 0;
	}

	/* configure the QSPI I/O interface */
	qspi_io_init(qspi_io_ctx, ctx->source_clock);

	if ((ctx->sfdp_skip == false) && sfdp_discover(&sfdp_info, ctx, (sfdp_read_cb_t) qspi_flash_sfdp_read)) {
	    int ret;
	    int erase_table_entries;
	    uint8_t read_instruction;
	    uint8_t write_instruction;

	    ctx->sfdp_supported = true;

	    /* Parameters from SFDP used:
	     * 1) Flash size.
	     * 2) Page size.
	     * 3) Address bytes - 3, 4, or both
	     *    Set the "current" address bytes to 3 if it's 3 only.
	     *    Otherwise set to 4.
	     *    If both modes are allowed, then should switch to 4 byte mode.
	     * 4) Supports_144_fast_read. This is required. Ensure that its command
	     *    is 0xEB.
	     * 5) Ensure that the quad i/o read's mode plus dummy clocks equals 6.
	     * 6) All the erase sizes and commands. Sort them and save to a table
	     *    inside the flash ctx.
	     * 7) The busy poll method.
	     * 8) The quad enable method.
	     *
	     * TODO:
	     * 9) If XIP mode is supported, The XIP entry/exit methods and implement them.
	     *    Should have xip enter/xip exit functions. Reads will need issue the
	     *    proper mode bits. Continue to use the xip read function?
	     */

	    /* Verify that the QSPI flash chip supports quad I/O read mode with 6 dummy cycles */
	    xassert(sfdp_info.basic_parameter_table.supports_144_fast_read && "Quad I/O Read mode support is required");
	    xassert(sfdp_info.basic_parameter_table.quad_144_read_cmd == QUAD_IO_READ_CMD_VAL && "Unsupported Quad I/O Read command");
	    xassert(sfdp_info.basic_parameter_table.quad_144_read_mode_clocks + sfdp_info.basic_parameter_table.quad_144_read_dummy_clocks == 6 && "Unsupported number of dummy clocks");

	    /* Save the page and flash sizes. Calculate the page count */
	    ctx->page_size_bytes = sfdp_flash_page_size_bytes(&sfdp_info);
	    ctx->flash_size_kbytes = sfdp_flash_size_kbytes(&sfdp_info);
	    if (ctx->flash_size_kbytes) {
	        ctx->page_count = (ctx->flash_size_kbytes >> sfdp_info.basic_parameter_table.page_size) << 10;
	    }
	    xassert(ctx->flash_size_kbytes != 0 && "Unsupported flash size");

	    /* Save the supported busy poll method */
	    ret = sfdp_busy_poll_method(&sfdp_info, &read_instruction, &ctx->busy_poll_bit, &ctx->busy_poll_ready_value);
	    if (ret == 0) {
	        ctx->busy_poll_cmd = QSPI_IO_BYTE_TO_MOSI(read_instruction);
	    }
	    xassert(ret == 0 && "Unsupported busy poll method");

	    /* Save the supported quad enable method */
	    ret = sfdp_quad_enable_method(&sfdp_info, &ctx->qe_reg, &ctx->qe_bit, &read_instruction, &write_instruction);
        if (ret == 0) {
            ctx->sr2_read_cmd = QSPI_IO_BYTE_TO_MOSI(read_instruction);
            ctx->sr2_write_cmd = QSPI_IO_BYTE_TO_MOSI(write_instruction);
        }
        xassert(ret == 0 && "Unsupported QE enable method");

        /* Parse and save the erase table */
        erase_table_entries = 0;
        for (int i = 0; i < 4; i++) {
            if (sfdp_info.basic_parameter_table.erase_info[i].size != 0) {
                ctx->erase_info[erase_table_entries].size_log2 = sfdp_info.basic_parameter_table.erase_info[i].size;
                ctx->erase_info[erase_table_entries].cmd = QSPI_IO_BYTE_TO_MOSI(sfdp_info.basic_parameter_table.erase_info[i].cmd);
            } else {
                ctx->erase_info[erase_table_entries].size_log2 = 0;
            }
            erase_table_entries++;
        }
        xassert(erase_table_entries > 0 && "Erase table found in SFDP is empty!");

        /* Determine the number of address bytes. If 4 byte mode is available, switch to it */
	    switch (sfdp_info.basic_parameter_table.address_bytes) {
	    case sfdp_3_or_4_byte_address:
	        ctx->address_bytes = 3; break; /* leave it in 3 byte address mode for now */

	        /* TODO: enable 4 byte address mode now and fall thru to next case */
	        // @suppress("No break at end of case")
	    case sfdp_4_byte_address:
	        ctx->address_bytes = 4;
	        xassert(0 && "4 byte address mode entry not yet implemented");
	        break;
        case sfdp_3_byte_address:
        default:
            ctx->address_bytes = 3;
            break;
	    }

	} else {
	    ctx->sfdp_supported = false;
	    debug_printf("Warning: QSPI flash does not support SFDP. Will use manually set parameters\n");
	    xassert((ctx->address_bytes == 3 || ctx->address_bytes == 4) && ctx->busy_poll_bit <= 7 && (ctx->busy_poll_ready_value == 0 || ctx->busy_poll_ready_value == 1));
	}
}
