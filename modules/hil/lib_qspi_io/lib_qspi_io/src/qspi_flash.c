// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "qspi_flash.h"
#if QSPI_FLASH_SANITY_CHECKS
	/*
	 * Ensure NDEBUG is not defined when the
	 * flash sanity checks are enabled.
	 */
	#if defined(NDEBUG)
		#undef NDEBUG
	#endif
#endif
#include <xcore/assert.h>
#include <xcore/hwtimer.h>

#define BARRIER() asm volatile("": : :"memory")

#define WRITE_ENABLE_COMMAND      QSPI_IO_BYTE_TO_MOSI(0x06)
#define WRITE_DISABLE_COMMAND     QSPI_IO_BYTE_TO_MOSI(0x04)

#define ERASE_4K_COMMAND          QSPI_IO_BYTE_TO_MOSI(0x20)
#define ERASE_32K_COMMAND         QSPI_IO_BYTE_TO_MOSI(0x52)
#define ERASE_64K_COMMAND         QSPI_IO_BYTE_TO_MOSI(0xD8)
#define ERASE_CHIP_COMMAND        QSPI_IO_BYTE_TO_MOSI(0xC7)

#define PAGE_PROGRAM_COMMAND      QSPI_IO_BYTE_TO_MOSI(0x02)

#define READ_STATUS_REG_COMMAND   QSPI_IO_BYTE_TO_MOSI(0x05)
#define READ_ID_COMMAND           QSPI_IO_BYTE_TO_MOSI(0x9F)

#define WRITE_STATUS_REG_COMMAND  QSPI_IO_BYTE_TO_MOSI(0x01)

#define QUAD_IO_READ_COMMAND      QSPI_IO_BYTE_TO_MOSI(0xEB)

#define QUAD_IO_READ_DUMMY_CYCLES 4

void qspi_flash_write_enable(qspi_flash_ctx_t *ctx)
{
	qspi_io_ctx_t *qspi_io_ctx = &ctx->qspi_io_ctx;

	qspi_io_start_transaction(qspi_io_ctx, WRITE_ENABLE_COMMAND, 8, qspi_io_full_speed);
	qspi_io_end_transaction(qspi_io_ctx);
}

void qspi_flash_write_disable(qspi_flash_ctx_t *ctx)
{
	qspi_io_ctx_t *qspi_io_ctx = &ctx->qspi_io_ctx;

	qspi_io_start_transaction(qspi_io_ctx, WRITE_DISABLE_COMMAND, 8, qspi_io_full_speed);
	qspi_io_end_transaction(qspi_io_ctx);
}

int qspi_flash_write_in_progress(qspi_flash_ctx_t *ctx)
{
	uint8_t status_reg;
	qspi_flash_read_status_register(ctx, &status_reg, 1);
	
	return (status_reg & QSPI_FLASH_STATUS_REG_WIP_BM) != 0;
}

void qspi_flash_wait_while_write_in_progress(qspi_flash_ctx_t *ctx)
{
	qspi_flash_poll_status_register(ctx, QSPI_FLASH_STATUS_REG_WIP_BM, 0);
}

void qspi_flash_erase(qspi_flash_ctx_t *ctx,
                      uint32_t address,
                      qspi_flash_erase_length_t erase_length)
{
	qspi_io_ctx_t *qspi_io_ctx = &ctx->qspi_io_ctx;

	size_t cycles = 32;
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

	switch (erase_length) {
	case qspi_flash_erase_4k:
		cmd = ERASE_4K_COMMAND;
		break;
	case qspi_flash_erase_32k:
		cmd = ERASE_32K_COMMAND;
		break;
	case qspi_flash_erase_64k:
		cmd = ERASE_64K_COMMAND;
		break;
	case qspi_flash_erase_chip:
		cmd = ERASE_CHIP_COMMAND;
		cycles = 8;
		break;
	default:
		xassert(0);
		return;
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
inline void qspi_flash_read_i(qspi_flash_ctx_t *ctx,
                              const qspi_io_transfer_mode_t transfer_mode,
                              const int xip,
                              uint8_t *data,
                              uint32_t address,
                              size_t len)
{
	qspi_io_ctx_t *qspi_io_ctx = &ctx->qspi_io_ctx;
	size_t cycles;
	size_t first_input_byte;
	size_t input_cycle;
	
	/* The first input should occur on either the fourth
	 * byte, or the last byte, whichever is first. */
	first_input_byte = len > 4 ? 4 : len;
	
	if (xip) {
		cycles = 8 + /* 8 cycles for address */
		         QUAD_IO_READ_DUMMY_CYCLES + /* dummy cycles */
		         2 * len; /* 2 cycles per byte */

		input_cycle = 8 + /* 8 cycles for address */
		              QUAD_IO_READ_DUMMY_CYCLES + /* dummy cycles */
		              2 * first_input_byte - 1; /* input on the last cycle of the first input byte */
	} else {
		cycles = 8 * 2 + /* 8 cycles each for command and address */
		         QUAD_IO_READ_DUMMY_CYCLES + /* dummy cycles */
		         2 * len; /* 2 cycles per byte */

		input_cycle = 8 * 2 + /* 8 cycles each for command and address */
		              QUAD_IO_READ_DUMMY_CYCLES + /* dummy cycles */
		              2 * first_input_byte - 1; /* input on the last cycle of the first input byte */
	}

	/*
	 * The address is really contained in the upper 24 bits.
	 * The lower 8 bits are essentially 2 dummy cycles.
	 * Rotate, such that the MSB of address is sent out during
	 * the first two dummy cycles. Some flashes use this to
	 * enter "performance" or "XIP" mode.
	 */
	address = (address << 8) | (address >> 24);

	/* 
	 * This just helps to ensure that the above calculations
	 * have completed before beginning the QSPI transaction.
	 * Specifically, we don't want input_cycle to get calculated
	 * between qspi_io_sio_direction_input() and qspi_io_bytes_in().
	 */
	BARRIER();

	if (xip) {
		qspi_io_start_transaction(qspi_io_ctx, address, cycles, qspi_io_full_speed);
	} else {
		qspi_io_start_transaction(qspi_io_ctx, QUAD_IO_READ_COMMAND, cycles, qspi_io_full_speed);
		qspi_io_words_out(qspi_io_ctx, qspi_io_transfer_normal, &address, 1);
	}
	qspi_io_sio_direction_input(qspi_io_ctx);
	qspi_io_bytes_in(qspi_io_ctx, transfer_mode, data, input_cycle, len);
	qspi_io_end_transaction(qspi_io_ctx);
}

void qspi_flash_read(qspi_flash_ctx_t *ctx,
                     uint8_t *data,
                     uint32_t address,
                     size_t len)
{
	qspi_flash_read_i(ctx, qspi_io_transfer_normal, 0, data, address, len);
}

void qspi_flash_read_nibble_swapped(qspi_flash_ctx_t *ctx,
                                    uint8_t *data,
                                    uint32_t address,
                                    size_t len)
{
	qspi_flash_read_i(ctx, qspi_io_transfer_nibble_swap, 0, data, address, len);
}

void qspi_flash_xip_read(qspi_flash_ctx_t *ctx,
                         uint8_t *data,
                         uint32_t address,
                         size_t len)
{
	qspi_flash_read_i(ctx, qspi_io_transfer_normal, 1, data, address, len);
}

void qspi_flash_xip_read_nibble_swapped(qspi_flash_ctx_t *ctx,
                         uint8_t *data,
                         uint32_t address,
                         size_t len)
{
	qspi_flash_read_i(ctx, qspi_io_transfer_nibble_swap, 1, data, address, len);
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

	if (!ctx->quad_page_program_enable) {
        pp_cmd = PAGE_PROGRAM_COMMAND;
        cycles = 8  +  /* 8 cycles each for command */
                 24 +  /* 24 cycles for address */
                 8 * len; /* 8 cycles per byte */
	} else if (ctx->quad_page_program_enable == 1) {
        pp_cmd = ctx->quad_page_program_cmd;
        cycles = 8 +   /* 8 cycles each for command */
                 6 +   /* 6 cycles for address */
                 2 * len; /* 2 cycles per byte */
	} else {
        pp_cmd = ctx->quad_page_program_cmd;
        cycles = 8  +  /* 8 cycles each for command */
                 24 +  /* 24 cycles for address */
                 2 * len; /* 2 cycles per byte */
	}

	/*
	 * Manipulate the address so that in memory it is:
	 * {byte 2, byte 1, byte 0, XX}
	 * it will get sent out as 3 bytes.
	 */
	address = byterev(address << 8);
	address_bytes = (uint8_t *) &address;

	qspi_io_start_transaction(qspi_io_ctx, pp_cmd, cycles, qspi_io_full_speed);
	if (!ctx->quad_page_program_enable) {
        qspi_io_mosi_out(qspi_io_ctx, qspi_io_transfer_normal, address_bytes, 3);
        qspi_io_mosi_out(qspi_io_ctx, transfer_mode, data, len);
	} else if (ctx->quad_page_program_enable == 1) {
        qspi_io_bytes_out(qspi_io_ctx, qspi_io_transfer_normal, address_bytes, 3);
        qspi_io_bytes_out(qspi_io_ctx, transfer_mode, data, len);
	} else {
	    qspi_io_mosi_out(qspi_io_ctx, qspi_io_transfer_normal, address_bytes, 3);
	    qspi_io_bytes_out(qspi_io_ctx, transfer_mode, data, len);
	}
	qspi_io_end_transaction(qspi_io_ctx);
}

void qspi_flash_write(qspi_flash_ctx_t *ctx,
                      const uint8_t *data,
                      uint32_t address,
                      size_t len)
{
	qspi_flash_write_i(ctx, qspi_io_transfer_normal, data, address, len);
}

void qspi_flash_write_nibble_swapped(qspi_flash_ctx_t *ctx,
                      const uint8_t *data,
                      uint32_t address,
                      size_t len)
{
	qspi_flash_write_i(ctx, qspi_io_transfer_nibble_swap, data, address, len);
}

void qspi_flash_deinit(qspi_flash_ctx_t *ctx)
{
	qspi_io_ctx_t *qspi_io_ctx = &ctx->qspi_io_ctx;

	qspi_io_deinit(qspi_io_ctx);
}

void qspi_flash_init(qspi_flash_ctx_t *ctx)
{
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
}

