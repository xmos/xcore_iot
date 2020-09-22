// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <platform.h>
#include <timer.h>
#include <string.h>

#include "soc.h"
#include "xassert.h"
#include "sdram_dev.h"

static unsafe void sdram_handler(
        chanend ?data_to_dma_c,
        chanend ?data_from_dma_c,
        chanend ?ctrl_c,
        streaming chanend c_sdram)
{
    uint32_t cmd;
    s_sdram_state sdram_state;
    unsigned read_buffer[SDRAMCONF_READ_BUFFER_SIZE];
    unsigned write_buffer[SDRAMCONF_WRITE_BUFFER_SIZE];
    unsigned * movable read_buffer_pointer = read_buffer;
    unsigned * movable write_buffer_pointer = write_buffer;
    unsigned addr;
    unsigned word_len;

    sdram_init_state(c_sdram, sdram_state);

    while (1)
    {
        select
        {
        case !isnull(ctrl_c) => soc_peripheral_function_code_rx(ctrl_c, &cmd):
            switch( cmd )
            {
            case SDRAM_DEV_SHUTDOWN:
                sdram_shutdown( c_sdram );
                break;

            case SDRAM_DEV_WRITE:
                soc_peripheral_varlist_rx(
                        ctrl_c, 2,
                        sizeof(addr), &addr,
                        sizeof(word_len), &word_len);

                xassert(word_len <= SDRAMCONF_WRITE_BUFFER_SIZE);

                soc_peripheral_varlist_rx(
                        ctrl_c, 1,
                        word_len, write_buffer_pointer);

                sdram_write(c_sdram, sdram_state, addr, word_len, move(write_buffer_pointer));
                sdram_complete(c_sdram, sdram_state, write_buffer_pointer);

                /* TX something so the driver knows the write was completed */
                soc_peripheral_varlist_tx(
                        ctrl_c, 1,
                        sizeof(write_buffer_pointer[0]), &write_buffer_pointer);
                break;

            case SDRAM_DEV_READ:
//                memset( (char *)read_buffer_pointer, 0x00, SDRAMCONF_READ_BUFFER_SIZE );
                for(unsigned i=0;i<SDRAMCONF_READ_BUFFER_SIZE;i++){
                    read_buffer_pointer[i] = 0; //And clear read pointer
                }

                soc_peripheral_varlist_rx(
                        ctrl_c, 2,
                        sizeof(addr), &addr,
                        sizeof(word_len), &word_len);

                xassert(word_len <= SDRAMCONF_READ_BUFFER_SIZE);

                sdram_read (c_sdram, sdram_state, addr, word_len, move( read_buffer_pointer));
                sdram_complete(c_sdram, sdram_state, read_buffer_pointer);

                soc_peripheral_varlist_tx(
                        ctrl_c, 1,
                        word_len, read_buffer_pointer);
                break;

            default:
                /* Command is not valid */
                fail("default cmd\n");
                break;
            }
            break;
        }
    }
}


void sdram_dev(
        chanend ?data_to_dma_c,
        chanend ?data_from_dma_c,
        chanend ?ctrl_c,
        out buffered port:32 p_sdram_dq_ah,
        out buffered port:32 p_sdram_cas,
        out buffered port:32 p_sdram_ras,
        out buffered port:8 p_sdram_we,
        out port p_sdram_clk,
        clock sdram_cb_clk)
{
    streaming chan c_sdram[1];

    par {
        sdram_server(c_sdram, 1,
                     p_sdram_dq_ah,
                     p_sdram_cas,
                     p_sdram_ras,
                     p_sdram_we,
                     p_sdram_clk,
                     sdram_cb_clk,
                     SDRAMCONF_CAS_LATENCY,
                     SDRAMCONF_ROW_WORDS,
                     SDRAMCONF_COL_BITS,
                     SDRAMCONF_COL_ADDR_BITS,
                     SDRAMCONF_ROW_ADDR_BITS,
                     SDRAMCONF_BANK_ADDR_BITS,
                     SDRAMCONF_REFRESH_MS,
                     SDRAMCONF_REFRESH_CYCLES,
                     SDRAMCONF_CLOCK_DIVIDER);

        unsafe {
            sdram_handler(data_to_dma_c, data_from_dma_c, ctrl_c, c_sdram[0]);
        }
    }
}
