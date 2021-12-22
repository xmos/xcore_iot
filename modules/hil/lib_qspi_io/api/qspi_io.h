// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#pragma once

/** \file
 *  \brief API for QSPI I/O
 */

#include <stdlib.h> /* for size_t */
#include <stdint.h>
#include <xclib.h> /* for byterev() */
#include <xcore/port.h>
#include <xcore/clock.h>

/**
 * \addtogroup hil_qspi_io hil_qspi_io
 *
 * The public API for using HIL QSPI I/O.
 * @{
 */

/**
 * Enum type used to set which SCLK edge SIO is sampled on.
 */
typedef enum {
	qspi_io_sample_edge_rising = 0, /**< Sample SIO on the rising edge */
	qspi_io_sample_edge_falling     /**< Sample SIO on the falling edge */
} qspi_io_sample_edge_t;

/**
 * Enum type used to set which of the two clock sources SCLK is derived from.
 */
typedef enum {
	qspi_io_source_clock_ref = 0, /**< SCLK is derived from the 100 MHz reference clock */
	qspi_io_source_clock_xcore    /**< SCLK is derived from the core clock */
} qspi_io_source_clock_t;

/**
 * Enum type used to specify whether or not nibbles should be swapped during transfers.
 */
typedef enum {
	qspi_io_transfer_normal = 0, /**< Do not swap nibbles */
	qspi_io_transfer_nibble_swap /**< Swap nibbles */
} qspi_io_transfer_mode_t;

/**
 * Enum type used to specify whether the next transaction will be a full speed
 * QSPI transaction with dummy cycles, or a lower speed SPI read transaction
 * without dummy cycles.
 */
typedef enum {
	qspi_io_full_speed = 0, /**< The transaction will be full speed with dummy cycles */
	qspi_io_spi_read        /**< The transaction will be low speed without dummy cycles */
} qspi_io_transaction_type_t;

/**
 * This macro may be used when sending out bytes
 * that are only transmitted over the single data
 * line MOSI (SIO0). The returned word should be
 * transmitted using either qspi_io_start_transaction()
 * or qspi_io_words_out(). Typically the byte argument
 * to this macro is a constant known at compile time,
 * like commands, as the compiler can perform this
 * computation at compile time. For arrays of data,
 * it may be more appropriate to use qspi_io_mosi_out()
 * which more efficiently computes this transformation
 * at run time on the fly.
 *
 * When writing a single byte out in SPI mode,
 * the byte needs to be transformed such that
 * each nibble in the word that is sent out on
 * SIO contains one bit from the byte in bit 0
 * (which corresponds to SIO0, or MOSI).
 *
 * \param x The byte to send out to MOSI.
 */
#define QSPI_IO_BYTE_TO_MOSI(x)     ( \
((((x) >> 0) & 1) | 0xE) << (0 * 4) | \
((((x) >> 1) & 1) | 0xE) << (1 * 4) | \
((((x) >> 2) & 1) | 0xE) << (2 * 4) | \
((((x) >> 3) & 1) | 0xE) << (3 * 4) | \
((((x) >> 4) & 1) | 0xE) << (4 * 4) | \
((((x) >> 5) & 1) | 0xE) << (5 * 4) | \
((((x) >> 6) & 1) | 0xE) << (6 * 4) | \
((((x) >> 7) & 1) | 0xE) << (7 * 4) )


/**
 * The context structure that must be passed to each of the qspi_io functions.
 * Several of the members in this structure must be set by the application prior
 * to calling either qspi_io_init() or qspi_io_start_transaction().
 */
typedef struct {
	/**
	 * The clock block to use for the qspi_io interface.
	 *
	 * This must be set prior to calling qspi_io_init()
	 * and must not change.
	 */
	xclock_t clock_block;
	
	/**
	 * The chip select port. MUST be a 1-bit port.
	 *
	 * This must be set prior to calling qspi_io_init()
	 * and must not change.
	 */
	port_t cs_port;
	
	/**
	 * The SCLK port. MUST be a 1-bit port.
	 *
	 * This must be set prior to calling qspi_io_init()
	 * and must not change.
	 */
	port_t sclk_port;
	
	/**
	 * The SIO port. MUST be a 4-bit port.
	 *
	 * This must be set prior to calling qspi_io_init()
	 * and must not change.
	 */
	port_t sio_port;

	/**
	 * The divisor to use for QSPI reads and writes as well as SPI writes.
	 *
	 * The frequency of SCLK will be set to:
	 * (F_src) / (2 * full_speed_clk_divisor)
	 * Where F_src is the frequency of the source clock specified
	 * by the source_clock parameter of qspi_io_init().
	 *
	 * This must be set prior to the beginning of a transaction
	 * and may change between transactions.
	 */
	int full_speed_clk_divisor;

	/**
	 * The divisor to use for the clock when performing a SPI read. This
	 * may need to be slower than the clock used for writes and QSPI reads.
	 * This is because a small handful of instructions must execute to turn
	 * the SIO port around from output to input and they must execute within
	 * a single SCLK period during a SPI read. QSPI reads have dummy cycles
	 * where these instructions may execute which allows for a higher clock
	 * frequency.
	 *
	 * The frequency of SCLK will be set to:
	 * (F_src) / (2 * spi_read_clk_divisor)
	 * Where F_src is the frequency of the source clock specified
	 * by the source_clock parameter of qspi_io_init().
	 *
	 * This must be set prior to the beginning of a transaction
	 * and may change between transactions.
	 */
	int spi_read_clk_divisor;

	/**
	 * Number of SCLK cycles to delay the sampling of SIO on input
	 * during a full speed transaction. 
	 *
	 * Usually either 0 or 1 depending on the SCLK frequency.
	 *
	 * This must be set prior to the beginning of a transaction
	 * and may change between transactions.
	 */
	uint32_t full_speed_sclk_sample_delay;

	/**
	 * Number of SCLK cycles to delay the sampling of SIO on input
	 * during a SPI read transaction. 
	 *
	 * Usually either 0 or 1 depending on the SCLK frequency.
	 *
	 * This must be set prior to the beginning of a transaction
	 * and may change between transactions.
	 */
	uint32_t spi_read_sclk_sample_delay;

	/**
	 * The SCLK edge to sample the SIO input on during a full speed
	 * transaction. May be either qspi_io_sample_edge_rising or
	 * qspi_io_sample_edge_falling.
	 *
	 * This must be set prior to the beginning of a transaction
	 * and may change between transactions.
	 */
	qspi_io_sample_edge_t full_speed_sclk_sample_edge;

	/**
	 * The SCLK edge to sample the SIO input on during a SPI read
	 * transaction. May be either qspi_io_sample_edge_rising or
	 * qspi_io_sample_edge_falling.
	 *
	 * This must be set prior to the beginning of a transaction
	 * and may change between transactions.
	 */
	qspi_io_sample_edge_t spi_read_sclk_sample_edge;

	/**
	 * Number of core clock cycles to delay sampling the SIO pads during
	 * a full speed transaction. This allows for more fine grained adjustment
	 * of sampling time. The value may be between 0 and 5.
	 *
	 * This must be set prior to the beginning of a transaction
	 * and may change between transactions.
	 */
	uint32_t full_speed_sio_pad_delay;

	/**
	 * Number of core clock cycles to delay sampling the SIO pads during
	 * a SPI read transaction. This allows for more fine grained adjustment
	 * of sampling time. The value may be between 0 and 5.
	 *
	 * This must be set prior to the beginning of a transaction
	 * and may change between transactions.
	 */
	uint32_t spi_read_sio_pad_delay;

	/* The following are used internally and should not be set by the application */
	size_t   transaction_length;
	uint32_t transaction_start;
	uint32_t sample_delay;
	uint32_t sample_edge;
	uint32_t sio_pad_delay;
	uint32_t sio_drive;
} qspi_io_ctx_t;

/**
 * This function should only be called internally
 * by qspi_io_mosi_out(). It performs the same
 * transformation as QSPI_IO_BYTE_TO_MOSI() but
 * also integrates the byte reversal and nibble
 * swap performed by qspi_io_words_out().
 *
 * \param x The byte to send out to MOSI.
 *
 * \returns the word to output to SIO.
 */
__attribute__((always_inline))
inline uint32_t qspi_io_byte_to_mosi(uint32_t x)
{
	uint32_t ones = 0xFFFFFFFF;
	x |= 0xFFFFFF00;

	asm volatile (
		"zip %0, %1, 0\n"
		"zip %1, %0, 0\n"
		"bitrev %0, %0\n"
		:"+r"(x), "+r"(ones)
	);

	return x;
}

/**
 * This function should only be called internally
 * by qspi_io_miso_in().
 *
 * When reading in a single byte in SPI mode,
 * the word that is received on SIO needs to
 * be transformed such that bit one from each
 * nibble (which corresponds to SIO1, or MISO)
 * is shifted into the correct bit position.
 *
 * \param x The word received on SIO.
 *
 * \returns the byte received on MISO.
 */
__attribute__((always_inline))
inline uint32_t qspi_io_miso_to_byte(uint32_t x)
{
	uint32_t unzipped = 0;

	asm volatile (
		"bitrev %1, %1\n"
		"unzip %0, %1, 0\n"
		"unzip %0, %1, 0\n"
		: "+r"(unzipped), "+r"(x)
	);

	return unzipped;
}

/**
 * This function swaps the nibbles in each of the
 * four bytes of \p word.
 *
 * \param word The word to nibble swap.
 *
 * \returns the nibble swapped word.
 */
__attribute__((always_inline))
inline uint32_t qspi_io_nibble_swap(uint32_t word)
{
	uint32_t tmp;

	/* word = ((word & 0x0F0F0F0F) << 4) | ((word & 0xF0F0F0F0) >> 4) */
	asm volatile (
		"{and %0,%0,%2 ; and  %1,%0,%3}\n"
		"{shl %0,%0,4  ; shr  %1,%1,4}\n"
		: "+r"(word), "=r"(tmp)
		: "r"(0x0F0F0F0F), "r"(0xF0F0F0F0)
	);

	return word | tmp;
}

/* The SETC constant for pad delay is missing from xs2a_user.h */
#define QSPI_IO_SETC_PAD_DELAY(n) (0x7007 | ((n) << 3))

/* These appear to be missing from the public API of lib_xcore */
#define QSPI_IO_RESOURCE_SETCI(res, c) asm volatile( "setc res[%0], %1" :: "r" (res), "n" (c))
#define QSPI_IO_RESOURCE_SETC(res, r) asm volatile( "setc res[%0], %1" :: "r" (res), "r" (r))
#define QSPI_IO_SETSR(c) asm volatile("setsr %0" : : "n"(c));
#define QSPI_IO_CLRSR(c) asm volatile("clrsr %0" : : "n"(c));

/**
 * Begins a new QSPI I/O transaction by starting the clock,
 * asserting CS, and sending out the first word which is
 * typically a command.
 *
 * \note If more words or bytes need to be sent or received as
 * part of this transaction, then the appropriate functions will
 * need to be called immediately following this one. For example,
 * qspi_io_bytes_out() then qspi_io_sio_direction_input() then
 * qspi_io_bytes_in(). The "out" or "in" instruction in each must
 * be executed within eight SCLK cycles of the preceding one,
 * including the OUT instruction in qspi_io_start_transaction().
 * Some analysis may be necessary depending on the frequency of SCLK.
 * These functions are all marked as inline to help keep them closer
 * together by removing the overhead associated with function calls
 * and to allow better optimization.
 *
 * \note It is likely not possible to follow an input with an output
 * within a single transaction unless the frequency of SCLK is
 * sufficiently slow. Fortunately in practice this rarely, if ever,
 * required.
 *
 * \param ctx              Pointer to the QSPI I/O context.
 * \param first_word       The first word to output.
 * \param len              The total number of clock cycles in the transaction.
 *                         CS will at some point during the transaction be setup
 *                         to deassert automatically after this number of cycles.
 * \param transaction_type Set to qspi_io_spi_read if the transaction will be a SPI
 *                         read with no dummy cycles. This may run at a slower clock
 *                         frequency in order to turn around SIO from output to input
 *                         in time. Otherwise set to qspi_io_full_speed.
 */
__attribute__((always_inline))
inline void qspi_io_start_transaction(qspi_io_ctx_t *ctx,
                                      uint32_t first_word,
                                      size_t len,
                                      qspi_io_transaction_type_t transaction_type)
{
	/* enable fast mode and high priority */
	QSPI_IO_SETSR(XS1_SR_QUEUE_MASK | XS1_SR_FAST_MASK);

	ctx->transaction_length = len;

	port_set_master(ctx->sio_port);
	port_set_no_ready(ctx->sio_port);

	if (transaction_type != qspi_io_full_speed) {
		clock_set_divide(ctx->clock_block, ctx->spi_read_clk_divisor);
		ctx->sample_delay  = ctx->spi_read_sclk_sample_delay;
		ctx->sample_edge   = ctx->spi_read_sclk_sample_edge;
		ctx->sio_pad_delay = QSPI_IO_SETC_PAD_DELAY(ctx->spi_read_sio_pad_delay);
		ctx->sio_drive     = XS1_SETC_DRIVE_PULL_UP; /* enable pullups during the read */
	} else {
		clock_set_divide(ctx->clock_block, ctx->full_speed_clk_divisor);
		ctx->sample_delay  = ctx->full_speed_sclk_sample_delay;
		ctx->sample_edge   = ctx->full_speed_sclk_sample_edge;
		ctx->sio_pad_delay = QSPI_IO_SETC_PAD_DELAY(ctx->full_speed_sio_pad_delay);
		ctx->sio_drive     = XS1_SETC_DRIVE_DRIVE; /* disable pullups during the read */
	}

	first_word = byterev(first_word);
	first_word = qspi_io_nibble_swap(first_word);

	/* ensure pullups are disabled during output */
	QSPI_IO_RESOURCE_SETCI(ctx->sio_port, XS1_SETC_DRIVE_DRIVE);

	port_out(ctx->sio_port, first_word);
	port_out(ctx->cs_port, 1);
	clock_start(ctx->clock_block);
}

/**
 * Outputs a byte array to the QSPI interface. The byte array
 * must start on a 4-byte boundary. This call must be made in
 * time such that its OUT instruction executes within 8 SCLK
 * cycles of the previous OUT instruction.
 *
 * \param ctx           Pointer to the QSPI I/O context.
 * \param transfer_mode Can be either qspi_io_transfer_normal or qspi_io_transfer_nibble_swap.
 *                      When qspi_io_transfer_nibble_swap, each byte transferred out is
 *                      nibble swapped. Because the data is inherently sent out nibble swapped
 *                      over the port, setting this to qspi_io_transfer_nibble_swap
 *                      actually removes a nibble swap operation.
 * \param data          Pointer to the byte array to output. This MUST
 *                      begin on a 4-byte boundary.
 * \param len           The number of bytes in \p data to output.
 */
__attribute__((always_inline))
inline void qspi_io_bytes_out(const qspi_io_ctx_t *ctx,
                              const qspi_io_transfer_mode_t transfer_mode,
                              const uint8_t *data,
                              size_t len)
{
	const uint32_t *data_words = (const uint32_t *) data;
	size_t word_len = len / sizeof(uint32_t);
	uint32_t word;
	int i;

	/*
	 * Each iteration of this loop must execute within
	 * no more than eight SCLK cycles.
	 */
	for (i = 0; i < word_len; i++) {
		word = data_words[i];
		if (transfer_mode == qspi_io_transfer_normal) {
			word = qspi_io_nibble_swap(word);
		}
		port_out(ctx->sio_port, word);
	}
	
	len &= sizeof(uint32_t) - 1; /* get the byte remainder */
	
	if (len) {
		word = data_words[i];
		len *= 8;

		if (transfer_mode == qspi_io_transfer_normal) {
			word = qspi_io_nibble_swap(word);
		}
		port_set_shift_count(ctx->sio_port, len);
		port_out(ctx->sio_port, word);
	}
}

/**
 * Outputs a word array to the QSPI interface. This call must be
 * made in time such that its OUT instruction executes within 8 SCLK
 * cycles of the previous OUT instruction.
 *
 * \param ctx           Pointer to the QSPI I/O context.
 * \param transfer_mode Can be either qspi_io_transfer_normal or qspi_io_transfer_nibble_swap.
 *                      When qspi_io_transfer_nibble_swap, each byte transferred out is
 *                      nibble swapped. Because the data is inherently sent out nibble swapped
 *                      over the port, setting this to qspi_io_transfer_nibble_swap
 *                      actually removes a nibble swap operation.
 * \param data          Pointer to the word array to output.
 * \param len           The number of words in \p data to output.
 */
__attribute__((always_inline))
inline void qspi_io_words_out(const qspi_io_ctx_t *ctx,
                              const qspi_io_transfer_mode_t transfer_mode,
                              const uint32_t *data,
                              size_t len)
{
	uint32_t word;

	/*
	 * Each iteration of this loop must execute within
	 * no more than eight SCLK cycles.
	 */
	for (int i = 0; i < len; i++) {
		word = byterev(data[i]);
		if (transfer_mode == qspi_io_transfer_normal) {
			word = qspi_io_nibble_swap(word);
		}
		port_out(ctx->sio_port, word);
	}
}

/**
 * Outputs a byte array to the QSPI interface over the single data
 * line MOSI (SIO0). This call must be made in time such that its
 * OUT instruction executes within 8 SCLK cycles of the previous OUT
 * instruction.
 *
 * \param ctx           Pointer to the QSPI I/O context.
 * \param transfer_mode Can be either qspi_io_transfer_normal or qspi_io_transfer_nibble_swap.
 *                      When qspi_io_transfer_nibble_swap, each byte transferred out is
 *                      nibble swapped.
 * \param data          Pointer to the byte array to output.
 * \param len           The number of words in \p data to output.
 */
__attribute__((always_inline))
inline void qspi_io_mosi_out(const qspi_io_ctx_t *ctx,
                             const qspi_io_transfer_mode_t transfer_mode,
                             const uint8_t *data,
                             size_t len)
{
	uint32_t word;

	/*
	 * Each iteration of this loop must execute within
	 * no more than eight SCLK cycles.
	 */
	for (int i = 0; i < len; i++) {
		word = data[i];
		if (transfer_mode != qspi_io_transfer_normal) {
			word = qspi_io_nibble_swap(word);
		}
		word = qspi_io_byte_to_mosi(word);
		port_out(ctx->sio_port, word);
	}
}

/**
 * This must be called to change the direction of SIO from output
 * to input before calling either qspi_io_bytes_in() or qspi_io_words_in().
 * This call must be made in time such that the call to port_set_buffered()
 * completes before the sample edge of SCLK shifts in the first nibble of
 * the next data word to be read.
 * This also will setup CS to deassert at the end of the transaction
 * while waiting for the previous output to complete.
 *
 * \note This is probably the most fragile function. Ensure that the port
 * direction is turned around on time, and that the subsequent read IN
 * instruction executes on time, by inspecting a VCD trace with both ports
 * and instructions.
 *
 * \param ctx Pointer to the QSPI I/O context.
 */
__attribute__((always_inline))
inline void qspi_io_sio_direction_input(qspi_io_ctx_t *ctx)
{
	/*
	 * If CS has not yet been setup to deassert at the end of
	 * the transaction then do that now. It is happening here
	 * because there should still be time while waiting for the
	 * previous output to complete.
	 *
	 * This cannot be done at the end of a transaction after
	 * the input begins, as it will not be able to complete
	 * in time after the last IN instruction.
	 */
	if (ctx->transaction_length != 0) {
		uint32_t cs_start;
		port_sync(ctx->cs_port);
		cs_start = port_get_trigger_time(ctx->cs_port);
		port_out_at_time(ctx->cs_port, cs_start + ctx->transaction_length, 0);
		ctx->transaction_length = 0;
		ctx->transaction_start = cs_start + ctx->sample_delay;
	}

	if (ctx->sample_delay) {
		/*
		 * Applying a 5 cycle pad delay to the CS pin when delaying the sample
		 * time by one SCLK cycle helps to ensure that CS is still seen as
		 * active at that time, which is necessary since CS is SIO's ready signal.
		 * This does only work at higher frequencies (~50+ MHz), but these are the
		 * frequencies that require sampling be delayed by a clock cycle.
		 */
		QSPI_IO_RESOURCE_SETCI(ctx->cs_port, QSPI_IO_SETC_PAD_DELAY(5));
	}

	/*
	 * This mess is to ensure that immediately following the sync,
	 * the port is put into buffered mode and the sample edge is
	 * set to falling if required as quickly as possible. Not sure
	 * at this time how to achieve this without using inline assembly.
	 * If the syncr+setc instructions are not grouped together in a
	 * single asm volatile() group, then they get merged, resulting
	 * in a single syncr instruction, followed by checking sample_edge,
	 * then executing the appropriate setc instructions.
	 */
	if (ctx->sample_edge == qspi_io_sample_edge_falling) {
		//port_sync(ctx->sio_port);
		//port_reset(ctx->sio_port);
		//port_set_sample_falling_edge(ctx->sio_port);
		//port_set_buffered(ctx->sio_port);
		asm volatile (
			"syncr res[%0]\n"
			"setc res[%0], %1\n"
			"setc res[%0], %2\n"
			"setc res[%0], %3\n"
			: :
			"r"(ctx->sio_port),
			"n"(XS1_SETC_INUSE_ON),
			"n"(XS1_SETC_SDELAY_SDELAY),
			"n"(XS1_SETC_BUF_BUFFERS)
		);
	} else {
		//port_sync(ctx->sio_port);
		//port_reset(ctx->sio_port);
		//port_set_buffered(ctx->sio_port);
		asm volatile (
			"syncr res[%0]\n"
			"setc res[%0], %1\n"
			"setc res[%0], %2\n"
			: :
			"r"(ctx->sio_port),
			"n"(XS1_SETC_INUSE_ON),
			"n"(XS1_SETC_BUF_BUFFERS)
		);
	}

	QSPI_IO_RESOURCE_SETC(ctx->sio_port, ctx->sio_pad_delay);
	QSPI_IO_RESOURCE_SETC(ctx->sio_port, ctx->sio_drive);
	port_set_transfer_width(ctx->sio_port, 32);
	port_set_ready_strobed(ctx->sio_port);
	port_set_slave(ctx->sio_port);
}

/**
 * Inputs a byte array from the QSPI interface. The byte array
 * must start on a 4-byte boundary. qspi_io_sio_direction_input()
 * must have been called previously. This call must be made in time
 * such that its IN instruction executes before \p start_time.
 *
 * \note The number of bytes input may be any number. However, if
 * it is NOT a multiple of four, then this likely will need to be
 * the last call in the transaction. This is because the shorter
 * length of the last input chunk plus the extra overhead required
 * to deal with the sub-word accesses will not allow subsequent I/O
 * to keep up unless the SCLK frequency is significantly slower than
 * the core clock.
 *
 * \param ctx           Pointer to the QSPI I/O context.
 * \param transfer_mode Can be either qspi_io_transfer_normal or qspi_io_transfer_nibble_swap.
 *                      When qspi_io_transfer_nibble_swap, each byte transferred in is
 *                      nibble swapped. Because the data is inherently received nibble swapped
 *                      over the port, setting this to qspi_io_transfer_nibble_swap
 *                      actually removes a nibble swap operation.
 * \param data          Pointer to the byte array to save the received data to.
 *                      This MUST begin on a 4-byte boundary.
 * \param start_time    The port time, relative to the beginning of the transfer, 
 *                      at which to input the first group of four bytes. This must
 *                      line up with the last nibble of the fourth byte. If \p len
 *                      is less than four, then it must line up with the last nibble
 *                      of the last byte.
 * \param len           The number of bytes to input.
 */
__attribute__((always_inline))
inline void qspi_io_bytes_in(const qspi_io_ctx_t *ctx,
                             const qspi_io_transfer_mode_t transfer_mode,
                             uint8_t *data, 
                             uint32_t start_time,
                             size_t len)
{
	uint32_t *data_words = (uint32_t *) data;
	size_t word_len = len / sizeof(uint32_t);
	uint32_t word;
	int i;

	port_set_trigger_time(ctx->sio_port, ctx->transaction_start + start_time);

	/*
	 * Each iteration of this loop must execute within
	 * no more than eight SCLK cycles.
	 */
	for (i = 0; i < word_len; i++) {
		word = port_in(ctx->sio_port);
		if (transfer_mode == qspi_io_transfer_normal) {
			word = qspi_io_nibble_swap(word);
		}
		data_words[i] = word;
	}

	/* 
	 * Note: Some of the following code, including the final IN
	 * instruction, may execute well after the data has already
	 * shifted in. This is ok provided this is the end of the
	 * transaction. Also note that the SETPSC and IN instructions
	 * executing after the data has already shifted in only works
	 * correctly when the port is in strobed slave mode and the ready
	 * signal is deasserted immediately following the last byte. This
	 * ensures that the data stops shifting in at the end of the
	 * transaction and that the number of bits shifted in after the last
	 * full word matches the number of remaining bytes to be read in.
	 *
	 * None of this not applies if len is a multiple of four.
	 */

	len &= sizeof(uint32_t) - 1; /* get the byte remainder */
	
	if (len) {
		if (word_len > 0) {
			/*
			 * There appears to be a problem (bug?) where if there is
			 * a port time set on a port, and then a port shift
			 * count is set within one or two cycles prior to this
			 * port time, then the subsequent IN instruction hangs
			 * indefinitely. If only either a port time or a port
			 * shift count is used, there appears to be no problem.
			 * So we only set the port shift count here if there
			 * is not an active port time.
			 */
			port_set_shift_count(ctx->sio_port, len * 8);
		}

		word = port_in(ctx->sio_port);
		word >>= 8 * (4 - len);
		
		data = (uint8_t *) &data_words[i];
		if (transfer_mode == qspi_io_transfer_normal) {
			word = qspi_io_nibble_swap(word);
		}
		
		for (i = 0; i < len; i++) {
			*data++ = word & 0xFF;
			word >>= 8;
		}
	}
}

/**
 * Inputs a word array from the QSPI interface. qspi_io_sio_direction_input()
 * must have been called previously. This call must be made in time such
 * that its IN instruction executes before \p start_time.
 *
 * \param ctx           Pointer to the QSPI I/O context.
 * \param transfer_mode Can be either qspi_io_transfer_normal or qspi_io_transfer_nibble_swap.
 *                      When qspi_io_transfer_nibble_swap, each byte transferred in is
 *                      nibble swapped. Because the data is inherently received nibble swapped
 *                      over the port, setting this to qspi_io_transfer_nibble_swap
 *                      actually removes a nibble swap operation.
 * \param data          Pointer to the word array to save the received data to.
 * \param start_time    The time, relative to the beginning of the transfer, at which
 *                      to input the first word. This must line up with the last nibble
 *                      of the first word.
 * \param len           The number of words to input.
 */
__attribute__((always_inline))
inline void qspi_io_words_in(const qspi_io_ctx_t *ctx,
                             const qspi_io_transfer_mode_t transfer_mode,
                             uint32_t *data,
                             uint32_t start_time,
                             size_t len)
{
	uint32_t word;

	port_set_trigger_time(ctx->sio_port, ctx->transaction_start + start_time);

	/*
	 * Each iteration of this loop must execute within
	 * no more than eight SCLK cycles.
	 */
	for (int i = 0; i < len; i++) {
		word = port_in(ctx->sio_port);
		if (transfer_mode == qspi_io_transfer_normal) {
			word = qspi_io_nibble_swap(word);
		}
		data[i] = byterev(word);
	}
}

/**
 * Inputs a byte array from the QSPI interface over the single data
 * line MISO (SIO1). qspi_io_sio_direction_input() must have been
 * called previously. This call must be made in time such that its
 * IN instruction executes before \p start_time.
 *
 * \param ctx           Pointer to the QSPI I/O context.
 * \param transfer_mode Can be either qspi_io_transfer_normal or qspi_io_transfer_nibble_swap.
 *                      When qspi_io_transfer_nibble_swap, each byte transferred in is
 *                      nibble swapped.
 * \param data          Pointer to the byte array to save the received data to.
 * \param start_time    The time, relative to the beginning of the transfer, at which
 *                      to input the first byte. This must line up with the last bit
 *                      of the first byte.
 * \param len           The number of words to input.
 */
__attribute__((always_inline))
inline void qspi_io_miso_in(const qspi_io_ctx_t *ctx,
                            const qspi_io_transfer_mode_t transfer_mode,
                            uint8_t *data,
                            uint32_t start_time,
                            size_t len)
{
	uint32_t word;

	port_set_trigger_time(ctx->sio_port, ctx->transaction_start + start_time);

	/*
	 * Each iteration of this loop must execute within
	 * no more than eight SCLK cycles.
	 */
	for (int i = 0; i < len; i++) {
		word = qspi_io_miso_to_byte(port_in(ctx->sio_port));
		if (transfer_mode != qspi_io_transfer_normal) {
			word = qspi_io_nibble_swap(word);
		}
		data[i] = word;
	}
}

/**
 * Polls the SPI interface by repeatedly receiving a byte over MISO until
 * a specified condition is met. For each time the received byte does not meet
 * the condition, the deassertion of CS is extended by eight SCLK cycles.
 * qspi_io_sio_direction_input() must have been called previously. This call
 * must be made in time such that its IN instruction executes before \p start_time.
 *
 * \param ctx         Pointer to the QSPI I/O context.
 * \param mask        The bitmask to apply to the received byte before comparing
 *                    it to \p val;
 * \param val         The value that the received byte, masked with \p mask, must
 *                    match before returning.
 * \param start_time  The time, relative to the beginning of the transfer, at which
 *                    to input the first byte. This must line up with the last bit
 *                    of the first byte.
 */
__attribute__((always_inline))
inline void qspi_io_miso_poll(const qspi_io_ctx_t *ctx,
                              const uint8_t mask,
                              const uint8_t val,
                              uint32_t start_time)
{
	uint32_t word;
	
	port_set_trigger_time(ctx->sio_port, ctx->transaction_start + start_time);

	/*
	 * Each iteration of this loop must execute within
	 * no more than eight SCLK cycles.
	 */
	while (1) {
		word = qspi_io_miso_to_byte(port_in(ctx->sio_port));

		if ((word & mask) == val) {
			break;
		}

		port_clear_buffer(ctx->cs_port);
		port_out_at_time(ctx->cs_port, port_get_trigger_time(ctx->cs_port) + 8, 0);
	}
}

/**
 * This sets up CS to deassert at the end of the transaction if it has
 * not already, waits for the current QSPI transaction to complete, and
 * then stops SCLK.
 *
 * \param ctx Pointer to the QSPI I/O context.
 */
__attribute__((always_inline))
inline void qspi_io_end_transaction(const qspi_io_ctx_t *ctx)
{
	/*
	 * If the transaction included input, then CS should already
	 * have been setup to deassert at the end. If not, then do it
	 * now as there should still be time while waiting for the
	 * previous output to complete. A sync on CS is necessary in
	 * case this is immediately following a call to
	 * qspi_io_start_transaction().
	 */
	if (ctx->transaction_length != 0) {
		uint32_t cs_start;
		port_sync(ctx->cs_port);
		cs_start = port_get_trigger_time(ctx->cs_port);
		port_out_at_time(ctx->cs_port, cs_start + ctx->transaction_length, 0);
	}

	port_sync(ctx->cs_port);
	clock_stop(ctx->clock_block);

	/*
	 * Ensure the SIO port is back to being sampled on the falling
	 * edge with no pad delay, and that the CS port has no pad delay.
	 */
	QSPI_IO_RESOURCE_SETCI(ctx->sio_port, QSPI_IO_SETC_PAD_DELAY(0));
	QSPI_IO_RESOURCE_SETCI(ctx->cs_port, QSPI_IO_SETC_PAD_DELAY(0));
	port_set_sample_falling_edge(ctx->sio_port);
	
	/*
	 * Also enable pull-ups on the SIO lines between transactions. If the
	 * transaction ended with an input, then SIO is still in input mode so
	 * this prevents them from floating.
	 */
	QSPI_IO_RESOURCE_SETCI(ctx->sio_port, XS1_SETC_DRIVE_PULL_UP);

	/* disable fast mode and high priority */
	QSPI_IO_CLRSR(XS1_SR_QUEUE_MASK | XS1_SR_FAST_MASK);
}

/**
 * This disables and frees the clock block and all the ports associated with
 * the QSPI I/O interface.
 *
 * \param ctx Pointer to the QSPI I/O context. This should have been
 *            previously initialized with qspi_io_init().
 */
void qspi_io_deinit(const qspi_io_ctx_t *ctx);

/**
 * This sets up the clock block and all the ports associated with
 * the QSPI I/O interface. This must be called first prior to any
 * other QSPI I/O function.
 *
 * \param ctx          Pointer to the QSPI I/O context. This must be initialized
 *                     with the clock block and ports to use.
 * \param source_clock Set to qspi_io_source_clock_ref to use the 100 MHz reference 
 *                     clock as the source for SCLK. Set to qspi_io_source_clock_xcore
 *                     to use the xcore clock.
 */
void qspi_io_init(const qspi_io_ctx_t *ctx,
                  qspi_io_source_clock_t source_clock);

/**@}*/ // END: addtogroup hil_qspi_io
