// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/*
NOTE: cycles must be >= 5
void burn_cycles(uint32_t cycles);
*/

#define NSTACKWORDS     (0)
#define FUNCTION_NAME   burn_cycles

#define cycles          r0

.text
.issue_mode single
.align 4

.cc_top FUNCTION_NAME.function,FUNCTION_NAME

FUNCTION_NAME:
    entsp NSTACKWORDS
    sub cycles, cycles, 4
    shr cycles, cycles, 1

.L_loop_top:
    sub cycles, cycles, 1
    bt cycles, .L_loop_top

.L_finish:
    retsp NSTACKWORDS

.L_func_end:
.cc_bottom FUNCTION_NAME.function


.global FUNCTION_NAME
.type FUNCTION_NAME,@function
.set FUNCTION_NAME.nstackwords,NSTACKWORDS;     .global FUNCTION_NAME.nstackwords
.set FUNCTION_NAME.maxcores,1;                  .global FUNCTION_NAME.maxcores
.set FUNCTION_NAME.maxtimers,0;                 .global FUNCTION_NAME.maxtimers
.set FUNCTION_NAME.maxchanends,0;               .global FUNCTION_NAME.maxchanends
.size FUNCTION_NAME, .L_func_end - FUNCTION_NAME
