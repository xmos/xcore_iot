// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdint.h>

#include "qspi_io.h"

void qspi_io_deinit(const qspi_io_ctx_t *ctx)
{
	port_disable(ctx->cs_port);
	port_disable(ctx->sio_port);
	port_disable(ctx->sclk_port);
	clock_disable(ctx->clock_block);
}

void qspi_io_init(const qspi_io_ctx_t *ctx,
                  qspi_io_source_clock_t source_clock)
{
	/* Setup the clock block */
	clock_enable(ctx->clock_block);
	if (source_clock == qspi_io_source_clock_ref) {
		clock_set_source_clk_ref(ctx->clock_block);
	} else {
		clock_set_source_clk_xcore(ctx->clock_block);
	}


	/* Setup the chip select port */
	port_enable(ctx->cs_port);
	port_set_clock(ctx->cs_port, ctx->clock_block);
	/*
	 * CS is used internally as the ready signal for SIO, so it must be
	 * driven high when it is asserted. However, it must be active low
	 * on the chip pin itself for the external QSPI device. Therefore the
	 * CS port is put into invert mode.
	 * NOTE: invert mode is not modeled correctly in xsim VCD trace.
	 */
	port_set_invert(ctx->cs_port);
	/* Set chip select as the ready source for the clock */
	clock_set_ready_src(ctx->clock_block, ctx->cs_port);


	/* Setup the SIO port */
	port_enable(ctx->sio_port);

	/*
	 * Ensure SIO begins outputing high on all lines.
	 * This requires an active clock.
	 */
	port_set_clock(ctx->sio_port, XS1_CLKBLK_REF);
	port_out(ctx->sio_port, 0xF);
	port_sync(ctx->sio_port);

	/* Now set SIO to use the desired clock block. */
	port_set_clock(ctx->sio_port, ctx->clock_block);

	/*
	 * Always sample on the falling edge. This allows for
	 * faster SCLK frequencies, but in general works with
	 * any frequency that meets timing. This also ensures
	 * that the internal CS ready signal is captured on time
	 * at higher frequencies, so is important both when SIO
	 * is input and output.
	 */
	port_set_sample_falling_edge(ctx->sio_port);

	/*
	 * SIO is put into strobed slave mode. CS is already
	 * setup as the ready signal for SCLK. This ensures that
	 * the data does not begin shifting out out until CS
	 * is asserted.
	 */
	port_set_buffered(ctx->sio_port);
	port_set_transfer_width(ctx->sio_port, 32);
	port_set_ready_strobed(ctx->sio_port);
	port_set_slave(ctx->sio_port);

	/* Ensure the buffer is clear before the first transaction. */
	port_clear_buffer(ctx->sio_port);


	/* Setup the SCLK port */
	port_enable(ctx->sclk_port);
	port_set_clock(ctx->sclk_port, ctx->clock_block);
	port_set_out_clock(ctx->sclk_port);
}

