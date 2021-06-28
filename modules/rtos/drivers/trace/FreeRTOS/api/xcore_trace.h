// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef XCORE_TRACE_H_
#define XCORE_TRACE_H_

#if ENABLE_RTOS_XSCOPE_TRACE == 1

#include "FreeRTOSConfig.h"
#if configUSE_TRACE_FACILITY == 0
#error configUSE_TRACE_FACILITY must be enabled to trace
#endif

#include <xscope.h>

/* Set defaults config values */
#ifndef xcoretraceconfigXSCOPE_TRACE_BUFFER
#define xcoretraceconfigXSCOPE_TRACE_BUFFER         200
#endif

#ifndef xcoretraceconfigXSCOPE_TRACE_RAW_BYTES
#define xcoretraceconfigXSCOPE_TRACE_RAW_BYTES          0
#endif

#if( !( __XC__ ) )
#include "ascii_trace.h"
#endif /* __XC__ */

#endif /* ENABLE_RTOS_XSCOPE_TRACE */

#endif /* XCORE_TRACE_H_ */
