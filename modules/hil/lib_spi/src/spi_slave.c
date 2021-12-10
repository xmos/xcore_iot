// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <xs1.h>
#include <xclib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <xcore/clock.h>
#include <xcore/port.h>
#include <xcore/port_protocol.h>
#include <xcore/triggerable.h>
#include <xcore/interrupt_wrappers.h>
#include <xcore/interrupt.h>

#include "spi.h"

#define ASSERTED 1

typedef struct internal_ctx {
    const spi_slave_callback_group_t *spi_cbg;
    uint8_t* in_buf;
    uint8_t* in_buf_cur;
    uint8_t* in_buf_end;
    uint8_t* out_buf;
    uint8_t* out_buf_cur;
    uint8_t* out_buf_end;
    uint32_t cs_val;
    uint32_t running;
    port_t p_miso;
    port_t p_mosi;
    port_t p_cs;
    xclock_t cb_clk;
    int cpol;
    int cpha;
} internal_ctx_t;


__attribute__((always_inline))
static inline void receive_part_word(internal_ctx_t *ctx, uint32_t in_word, size_t *valid_bits) {
    uint32_t mask;
    uint32_t diff = ((ctx->in_buf_cur + 4) <= ctx->in_buf_end)
                    ? 4
                    : (ctx->in_buf_end - ctx->in_buf_cur);

    if (*valid_bits > 0) {
        asm volatile("mkmsk %0, %1": "=r"(mask) : "r"(*valid_bits));
        in_word = bitrev(in_word) & mask;

        /* Switch based on valid bits */
        uint32_t partial_shift_8 = *valid_bits - 8;
        uint32_t partial_shift_16 = *valid_bits - 16;
        uint32_t partial_shift_24 = *valid_bits - 24;
        if( *valid_bits <= 8 ) {
            if (diff >= 1) {
                *(ctx->in_buf_cur + 0) = (uint8_t)(in_word & 0xff);
            }
            if (diff >= 2) {
                *(ctx->in_buf_cur + 1) = (uint8_t)(0x00);
            }
            if (diff >= 3) {
                *(ctx->in_buf_cur + 2) = (uint8_t)(0x00);
            }
            if (diff >= 4) {
                *(ctx->in_buf_cur + 3) = (uint8_t)(0x00);
            }
        } else if (*valid_bits <= 16) {
            if (diff >= 1) {
                *(ctx->in_buf_cur + 0) = (uint8_t)((in_word>>partial_shift_8) & 0xff);
            }
            if (diff >= 2) {
                asm volatile("mkmsk %0, %1": "=r"(mask) : "r"(partial_shift_8));
                *(ctx->in_buf_cur + 1) = (uint8_t)((in_word) & mask);
            }
            if (diff >= 3) {
                *(ctx->in_buf_cur + 2) = (uint8_t)(0x00);
            }
            if (diff >= 4) {
                *(ctx->in_buf_cur + 3) = (uint8_t)(0x00);
            }
        } else if (*valid_bits <= 24) {
            if (diff >= 1) {
                *(ctx->in_buf_cur + 0) = (uint8_t)((in_word>>partial_shift_8) & 0xff);
            }
            if (diff >= 2) {
                *(ctx->in_buf_cur + 1) = (uint8_t)((in_word>>partial_shift_16) & 0xff);
            }
            if (diff >= 3) {
                asm volatile("mkmsk %0, %1": "=r"(mask) : "r"(partial_shift_16));
                *(ctx->in_buf_cur + 2) = (uint8_t)((in_word) & mask);
            }
            if (diff >= 4) {
                *(ctx->in_buf_cur + 3) = (uint8_t)(0x00);
            }
        } else {
            if (diff >= 1) {
                *(ctx->in_buf_cur + 0) = (uint8_t)((in_word>>partial_shift_8) & 0xff);
            }
            if (diff >= 2) {
                *(ctx->in_buf_cur + 1) = (uint8_t)((in_word>>partial_shift_16) & 0xff);
            }
            if (diff >= 3) {
                *(ctx->in_buf_cur + 2) = (uint8_t)((in_word>>partial_shift_24) & 0xff);
            }
            if (diff >= 4) {
                asm volatile("mkmsk %0, %1": "=r"(mask) : "r"(partial_shift_24));
                *(ctx->in_buf_cur + 3) = (uint8_t)((in_word) & mask);
            }
        }

        /* Increment in_buf_cur and update valid_bits depending on buffer space available  */
        uint32_t valid_bytes = (*valid_bits >> 3);
        if (valid_bytes < diff) {
            ctx->in_buf_cur += valid_bytes;
            *valid_bits %= 8;
        } else {
            ctx->in_buf_cur += diff;
            *valid_bits = 0;
        }
    }
}

__attribute__((always_inline))
static void inline receive_word(internal_ctx_t *ctx, uint32_t in_word) {
    in_word = bitrev(in_word);
    in_word = byterev(in_word);

    for (int i=0; i<4; i++)
    {
        if (ctx->in_buf_cur >= ctx->in_buf_end) {
            break;
        } else {
            *(ctx->in_buf_cur) = (uint8_t)(in_word);
            in_word >>= 8;
            ctx->in_buf_cur++;
        }
    }
}

__attribute__((always_inline))
static inline uint32_t get_next_word(internal_ctx_t *ctx) {
    uint32_t out_word = 0;

    if (ctx->out_buf_cur < ctx->out_buf_end)
    {
        out_word = *(uint32_t*)ctx->out_buf_cur;
        out_word = bitrev(out_word);
        out_word = byterev(out_word);
        switch(ctx->out_buf_end - ctx->out_buf_cur)
        {
            default:
            case 4:
                ctx->out_buf_cur += 4;
                break;
            case 3:
                out_word &= 0xFFFFFF00;
                ctx->out_buf_cur += 3;
                break;
            case 2:
                out_word &= 0xFFFF0000;
                ctx->out_buf_cur += 2;
                break;
            case 1:
                out_word &= 0xFF000000;
                ctx->out_buf_cur += 1;
                break;
        }
    }

    return out_word;
}

DEFINE_INTERRUPT_CALLBACK(spi_isr_grp, cs_isr, arg)
{
    internal_ctx_t* ctx = (internal_ctx_t*)arg;

    /* Update next CS event */
    ctx->cs_val = port_in(ctx->p_cs);
    port_set_trigger_value(ctx->p_cs, ctx->cs_val);

    if (ctx->cs_val == ASSERTED) {
        port_clear_buffer(ctx->p_mosi);

        if (ctx->p_miso != 0) {
            port_clear_buffer(ctx->p_miso);
        }

        size_t out_buf_len = 0;
        size_t in_buf_len = 0;
        ctx->spi_cbg->slave_transaction_started(ctx->spi_cbg->app_data, &ctx->out_buf, &out_buf_len, &ctx->in_buf, &in_buf_len);

        if (ctx->p_miso != 0) {
            ctx->out_buf_cur = ctx->out_buf;
            ctx->out_buf_end = (ctx->out_buf == NULL) ? NULL : (ctx->out_buf + out_buf_len);
        } else {
            ctx->out_buf_cur = NULL;
            ctx->out_buf_end = NULL;
        }

        ctx->in_buf_cur = ctx->in_buf;
        ctx->in_buf_end = (ctx->in_buf == NULL) ? NULL : (ctx->in_buf  + in_buf_len);

        ctx->running = 1;

        if (ctx->p_miso != 0) {
            uint32_t out_word = get_next_word(ctx);

            /* Must get first bit on the wire before clock edge */
            if (ctx->cpha == 0) {
                port_set_master(ctx->p_miso);
                port_set_no_ready(ctx->p_miso);
                port_set_clock(ctx->p_miso, XS1_CLKBLK_REF);

                spi_io_port_outpw(ctx->p_miso, out_word, 1);
                port_sync(ctx->p_miso);

                port_set_ready_strobed(ctx->p_miso);
                port_set_slave(ctx->p_miso);
                port_set_clock(ctx->p_miso, ctx->cb_clk);

                out_word >>= 1;
                spi_io_port_outpw(ctx->p_miso, out_word, 31);
            } else {
                port_out(ctx->p_miso, out_word);
            }

            out_word = get_next_word(ctx);

            port_out(ctx->p_miso, out_word);
        }
    } else {
        uint32_t data = 0;
        size_t read_bits = 0;
        uint32_t bytes_read = 0;
        uint32_t bytes_written = 0;

        read_bits = port_force_input(ctx->p_mosi, &data);
        receive_part_word(ctx, data, &read_bits);
        bytes_read = ctx->in_buf_cur - ctx->in_buf;
        port_clear_buffer(ctx->p_mosi);

        if (ctx->p_miso != 0) {
            bytes_written = ctx->out_buf_cur - ctx->out_buf;
            port_clear_buffer(ctx->p_miso);
        }

        ctx->running = 0;

        ctx->spi_cbg->slave_transaction_ended(ctx->spi_cbg->app_data, &ctx->out_buf, bytes_written, &ctx->in_buf, bytes_read, read_bits);
    }
}

void spi_slave(
        const spi_slave_callback_group_t *spi_cbg,
        port_t p_sclk,
        port_t p_mosi,
        port_t p_miso,
        port_t p_cs,
        xclock_t cb_clk,
        int cpol,
        int cpha) {
    internal_ctx_t int_ctx = {
        .p_miso = p_miso,
        .p_mosi = p_mosi,
        .p_cs = p_cs,
        .cb_clk = cb_clk,
        .cpol = cpol,
        .cpha = cpha,
        .spi_cbg = spi_cbg,
        .cs_val = !ASSERTED,
        .out_buf = NULL,
        .in_buf = NULL,
        .running = 0,
        .in_buf_end = NULL,
        .in_buf_cur = NULL,
        .out_buf_end = NULL,
        .out_buf_cur = NULL,
    };
    uint32_t in_word;
    uint32_t out_word;

	/* Enable fast mode and high priority */
	SPI_IO_SETSR(XS1_SR_QUEUE_MASK | XS1_SR_FAST_MASK);

    /* Setup the chip select port */
    port_enable(p_cs);
    port_set_invert(p_cs);

    /* Setup the SCLK port and associated clock block */
    port_enable(p_sclk);
    clock_enable(cb_clk);
    clock_set_source_port(cb_clk, p_sclk);
    clock_set_divide(cb_clk, 0);    /* Ensure divide is 0 */

    /* Setup the MOSI port */
    port_enable(p_mosi);
    port_protocol_in_strobed_slave(p_mosi, p_cs, cb_clk);
    port_set_transfer_width(p_mosi, 32);

    /* Setup the MISO port */
    if (p_miso != 0) {
        port_enable(p_miso);
        port_protocol_out_strobed_slave(p_miso, p_cs, cb_clk, 0);
        port_set_transfer_width(p_miso, 32);
    }

    if (cpol != cpha) {
        port_set_invert(p_sclk);
    } else {
        port_set_no_invert(p_sclk);
    }

    clock_start(cb_clk);

    port_sync(p_sclk);

    /* Wait until CS is not asserted to begin */
    int_ctx.cs_val = port_in_when_pinsneq(p_cs, PORT_UNBUFFERED, ASSERTED);

    triggerable_setup_interrupt_callback(p_cs, &int_ctx, INTERRUPT_CALLBACK(cs_isr));

    interrupt_mask_all();

    triggerable_enable_trigger(p_cs);
    port_set_trigger_in_not_equal(p_cs, int_ctx.cs_val);

    interrupt_unmask_all();

    while (1) {
        in_word = port_in(p_mosi);

        interrupt_mask_all();
        {
            asm volatile( "" ::: "memory" );
            if (int_ctx.running) {
                out_word = get_next_word(&int_ctx);
                receive_word(&int_ctx, in_word);
            }
        }
        interrupt_unmask_all();
        if (int_ctx.running) {
            if (p_miso != 0) {
                port_out(p_miso, out_word);
            }
        }
    }
}
