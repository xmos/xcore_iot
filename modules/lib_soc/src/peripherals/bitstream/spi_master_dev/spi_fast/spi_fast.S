// Copyright (c) 2020, XMOS Ltd, All rights reserved

/*
 * This function implements a SPI master.
 * All four modes (0,1,2,3) are supported.
 * An optional inter-byte delay is supported
 * for devices that require it.
 *
 * 50 MHz SPI clock with a system core clock
 * of 500 MHz is supported, though not guaranteed
 * if there are more than 6 cores (or 5 cores when
 * the interbyte delay is enabled). For this reason
 * the core is put into high priority mode.
 */

#define FUNCTION_NAME spi_fast_xfer
#define NSTACKWORDS  12

#include <xs2a_kernel.h>

#if __soc_conf_h_exists__
#include "soc_conf.h"
#endif
#include "spi_master_dev_conf_defaults.h"


/*
void spi_fast_xfer(
        unsigned num_bytes,
        char *buffer,
        spi_fast_ports *p,
        int save_input,
        uint32_t start_time,
        uint32_t end_time,
        unsigned cs_disable_value);
*/

.text
.issue_mode  dual
.globl FUNCTION_NAME
.align 4
.type FUNCTION_NAME,@function
.cc_top FUNCTION_NAME.function,FUNCTION_NAME

#define CLK_STRUCT_INDEX        0
#define MISO_STRUCT_INDEX       1
#define MOSI_STRUCT_INDEX       2
#define CS_STRUCT_INDEX         3
#define BYTE_SETUP_STRUCT_INDEX 7
#define CLK_BITS_STRUCT_INDEX   9
#define CLK_DELAY_STRUCT_INDEX  10

#define BYTE_SETUP_INDEX        8
#define CS_INDEX                9
#define CLK_DELAY_INDEX         10

#define START_TIME_INDEX  NSTACKWORDS + 1
#define END_TIME_INDEX    NSTACKWORDS + 2
#define CS_DISABLE_INDEX  NSTACKWORDS + 3


#define num_bytes    r0
#define buffer       r1
#define ports        r2
#define save_input   r3

#define clk          r4
#define miso         r5
#define mosi         r6
#define clk_bits     r2

#define tmp          r7
#define one          r8
#define zero         r9
#define tmp2         r10
#define _16          r11


.align 4
FUNCTION_NAME:
	/* enable fast mode and high priority */
	setsr XS1_SR_QUEUE_MASK | XS1_SR_FAST_MASK

	dualentsp NSTACKWORDS

	/* callee save registers r4-r10 are all used */
	stw r4,      sp[1]
	std r5, r6,  sp[1]
	std r7, r8,  sp[2]
	std r9, r10, sp[3]

    /* Get the some members out of the ports struct */
	ldw   clk,       ports[CLK_STRUCT_INDEX]
	ldw   miso,      ports[MISO_STRUCT_INDEX]
	ldw   mosi,      ports[MOSI_STRUCT_INDEX]
	ldw   tmp,       ports[BYTE_SETUP_STRUCT_INDEX]
	stw   tmp,       sp[BYTE_SETUP_INDEX]              /* save away byte_setup_ticks for later */
	ldw   tmp,       ports[CS_STRUCT_INDEX]            /* tmp gets the CS port */
	stw   tmp,       sp[CS_INDEX]                      /* save away the CS port for later */
	ldw   tmp2,      ports[CLK_DELAY_STRUCT_INDEX]     /* tmp2 gets clock_delay */
	stw   tmp2,      sp[CLK_DELAY_INDEX]               /* save away clock_delay for later */
	ldw   clk_bits,  ports[CLK_BITS_STRUCT_INDEX]

	/* some constants that get used each iteration */
	ldc   zero,      0
	ldc   one,       1
	ldc   _16,       16

	sub   buffer,    buffer,    1         /* the loop increments buffer before using it, so
	                                         begin with it backed up by one byte */

	ldw   tmp,       sp[START_TIME_INDEX] /* tmp gets start_time */

{	add   tmp2,      tmp,       tmp2      /* tmp2 gets start_time + clock_delay */
	setpt res[mosi], tmp                  /* port time for MOSI is start_time */
}

{	setpt res[clk],  tmp2                 /* port time for CLK is start_time + clock_delay */
	ld8u  tmp,       buffer[one]          /* read the first byte out of buffer */
}
	mov   tmp2,      tmp                  /* this "zips" the byte from buffer into 16 bits such that */
	zip   tmp2,      tmp,       0         /* 0b76543210 -> 0b7766554433221100 */

	outpw res[clk],  clk_bits,  16        /* output the 16 clock_pattern bits onto the clock port */
	outpw res[mosi], tmp,       16        /* output the 16 zipped data bits in tmp onto the mosi port */

#if SPICONF_INTERBYTE_DELAY_ENABLE
	ldw   tmp,       sp[BYTE_SETUP_INDEX];/* tmp gets byte_setup_ticks */
	bt    tmp,       ibd_skip1            /* skip the pre-loop port time setup of MISO if byte_setup_ticks
	                                         is non-zero since it will be done in the loop instead */
#endif
{	ldw   tmp,       sp[START_TIME_INDEX] /* tmp gets start_time */
	ldc   tmp2,      14                   /* need to add 14 to start_time */
}
	add   tmp2,      tmp,       tmp2      /* add 14 to start_time */
	setpt res[miso], tmp2                 /* port time for MISO is start_time + 14. this correctly lines up
	                                         sampling of MISO for all SPI modes */
ibd_skip1:
{	setpsc res[miso],_16
	eq    tmp,       num_bytes, 1
}

loop_start:

{	bt    tmp,       loop_end
	add   buffer,    buffer,    1
}
#if SPICONF_INTERBYTE_DELAY_ENABLE
	ldw   tmp,       sp[BYTE_SETUP_INDEX];/* tmp gets byte_setup_ticks */
	bf    tmp,       ibd_skip2            /* skip the loop port time setup of MISO if byte_setup_ticks
	                                         is zero since the timing is entirely driven by the initial port
	                                         time of MISO and the port shift count */
{	ldw   tmp,       sp[START_TIME_INDEX] /* tmp gets start_time */
	ldc   tmp2,      14                   /* need to add 14 to start_time */
}
	add   tmp2,      tmp,       tmp2      /* add 14 to start_time */
	setpt res[miso], tmp2                 /* port time for MISO is start_time + 14. this correctly lines up
	                                         sampling of MISO for all SPI modes */

{	add   tmp,       tmp,       _16       /* start_time += 16 */
	ldw   tmp2,      sp[BYTE_SETUP_INDEX];/* tmp2 gets byte_setup_ticks */
}
{	add   tmp,       tmp,       tmp2      /* start_time += byte_setup_ticks */
	ldw   tmp2,      sp[CLK_DELAY_INDEX]  /* tmp2 gets clock_delay */
}
{	add   tmp2,      tmp,       tmp2      /* tmp2 gets start_time + clock_delay */
	setpt res[mosi], tmp                  /* port time for MOSI is start_time */
}
{	stw   tmp,       sp[START_TIME_INDEX] /* save the updated start_time */
	setpt res[clk],  tmp2                 /* port time for CLK is start_time + clock_delay */
}
ibd_skip2:
#endif
{	ld8u  tmp,       buffer[one]          /* read the next byte out of buffer */
	sub   num_bytes, num_bytes, 1         /* decrement the byte counter */
}
	mov   tmp2,      tmp                  /* this "zips" the byte from buffer into 16 bits such that */
	zip   tmp2,      tmp, 0               /* 0b76543210 -> 0b7766554433221100 */

	outpw res[clk],  clk_bits,  16        /* output the 16 clock pattern bits onto the clock port */
	outpw res[mosi], tmp,       16        /* output the 16 zipped data bits in tmp onto the mosi port */

{	in    tmp2,      res[miso]            /* input MISO into tmp2 */
	eq    tmp,       num_bytes, 1         /* test the loop condition now. loop exits when num_bytes == 1 */
}
{	setpsc res[miso],_16                  /* set port shift count of MISO to 16 */
	bf    save_input,loop_start           /* go to the beginning of the loop if not saving the input */
}
	shr   tmp2,      tmp2,      16        /* input needs to be manipulated. start by shifting right 16 */
	unzip tmp,       tmp2,      0         /* "unzip" the input. tmp here is a don't care. resulting byte
	                                         from 16 bit input is 0b7x6x5x4x3x2x1x0x -> 0b76543210 */
	st8   tmp,       buffer[zero]         /* save this "unzipped" byte into buffer */
{	bu    loop_start                      /* go to the start of the loop */
	eq    tmp,       num_bytes, 1         /* test the loop condition now. loop exits when num_bytes == 1 */
}

loop_end:

#if SPICONF_INTERBYTE_DELAY_ENABLE
	ldw   tmp,       sp[BYTE_SETUP_INDEX];/* tmp gets byte_setup_ticks */
	bf    tmp,       ibd_skip3            /* skip the post-loop port time setup of MISO if byte_setup_ticks
	                                         is zero since the timing is entirely driven by the initial port
	                                         time of MISO and the port shift count. */
{	ldw   tmp,       sp[START_TIME_INDEX] /* tmp gets start_time */
	ldc   tmp2,      14                   /* need to add 14 to start_time */
}
	add   tmp2,      tmp,       tmp2      /* add 14 to start_time */
	setpt res[miso], tmp2                 /* port time for MISO is start_time + 14. this correctly lines up
	                                         sampling of MISO for all SPI modes */
ibd_skip3:
#endif

	ldw   tmp,       sp[END_TIME_INDEX]   /* get end_time into tmp. this is the time to de-assert CS */
	ldw   tmp2,      sp[CS_INDEX]         /* tmp2 gets the CS port */
{	setpt res[tmp2], tmp                  /* port time for CS is end_time */
	ldw   tmp,       sp[CS_DISABLE_INDEX] /* tmp gets cs_disable_value */
}
	out   res[tmp2], tmp                  /* write cs_disable_value to CS */

	in    tmp2,      res[miso]            /* input MISO into tmp2 */

	bf    save_input,restore_stack        /* skip saving the last input byte if not saving the input */

	shr   tmp2,      tmp2,      16        /* input needs to be manipulated. start by shifting right 16 */
	unzip tmp,       tmp2,      0         /* "unzip" the input. tmp here is a don't care. resulting byte
	                                         from 16 bit input is 0b7x6x5x4x3x2x1x0x -> 0b76543210 */
	st8   tmp,       buffer[zero]         /* save this last "unzipped" byte into buffer */

restore_stack:
	/* restore callee save registers r4-r10 */
	ldw r4,      sp[1]
	ldd r5, r6,  sp[1]
	ldd r7, r8,  sp[2]
	ldd r9, r10, sp[3]

	/* disable fast mode and high priority */
	clrsr XS1_SR_QUEUE_MASK | XS1_SR_FAST_MASK

.Lfunc_end:
	retsp NSTACKWORDS

	.cc_bottom FUNCTION_NAME.function
	.set FUNCTION_NAME.nstackwords,NSTACKWORDS
	.globl FUNCTION_NAME.nstackwords
	.set FUNCTION_NAME.maxcores,1
	.globl FUNCTION_NAME.maxcores
	.set FUNCTION_NAME.maxtimers,0
	.globl FUNCTION_NAME.maxtimers
	.set FUNCTION_NAME.maxchanends,0
	.globl FUNCTION_NAME.maxchanends
.Ltmp0:
	.size FUNCTION_NAME, .Ltmp0-FUNCTION_NAME
	.issue_mode  single
