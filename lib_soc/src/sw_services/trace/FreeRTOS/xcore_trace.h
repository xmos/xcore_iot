/*
 * xcore_trace.h
 *
 *  Created on: Sep 13, 2019
 *      Author: jmccarthy
 */


#ifndef XCORE_TRACE_H_
#define XCORE_TRACE_H_

#if __xcore_trace_config_h_exists__
#include "xcore_trace_config.h"
#endif

#include <xscope.h>

/* Set defaults config values */

#ifndef xcoretraceconfigXSCOPE_TRACE_BUFFER
#define xcoretraceconfigXSCOPE_TRACE_BUFFER         200
#endif

#ifndef xcoretraceconfigXSCOPE_TRACE_RAW_BYTES
#define xcoretraceconfigXSCOPE_TRACE_RAW_BYTES          0
#endif

#ifndef xcoretraceconfigSYSVIEW
#define xcoretraceconfigSYSVIEW 0
#endif

#ifndef xcoretraceconfigASCII
#define xcoretraceconfigASCII 0
#endif
/*-----------------------------------------------------------*/

#if !defined(configUSE_TRACE_FACILITY)

/* If the application has not defined configUSE_TRACE_FACILITY in
FreeRTOSConfig.h then set it here. It will be enabled if a trace
system is selected, otherwise it will be disabled. */

#if( ( xcoretraceconfigSYSVIEW == 1 ) || ( xcoretraceconfigASCII == 1 ) )
#define configUSE_TRACE_FACILITY            1
#else
#define configUSE_TRACE_FACILITY            0
#endif /* xcoretraceconfigSYSVIEW || xcoretraceconfigASCII */

#else /* !defined(configUSE_TRACE_FACILITY) */

#if configUSE_TRACE_FACILITY == 0

/* If the application has set configUSE_TRACE_FACILITY and it is 0,
then force disable all trace systems. */

#ifdef xcoretraceconfigSYSVIEW
#undef xcoretraceconfigSYSVIEW
#endif
#define xcoretraceconfigSYSVIEW 0

#ifdef xcoretraceconfigASCII
#undef xcoretraceconfigASCII
#endif
#define xcoretraceconfigASCII 0

#endif /* configUSE_TRACE_FACILITY == 0 */

#endif /* !defined(configUSE_TRACE_FACILITY) */
/*-----------------------------------------------------------*/

/* Do some error checking */

#if ( xcoretraceconfigSYSVIEW + xcoretraceconfigASCII ) > 1
#error You may only use one trace system

#elif ( xcoretraceconfigSYSVIEW == 1 ) && defined(_XSCOPE_PROBES_INCLUDE_FILE) && !defined(SEGGER_TRACE)
#error You must have an XSCOPE probe named SEGGER_TRACE when xcoretraceconfigSYSVIEW is enabled

#elif ( xcoretraceconfigASCII == 1 ) && defined(_XSCOPE_PROBES_INCLUDE_FILE) && !defined(FREERTOS_TRACE)
#error You must have an XSCOPE probe named FREERTOS_TRACE when xcoretraceconfigASCII is enabled

#else /* no errors */

#if( !( __XC__ ) )

/* Include support for the selected trace system */

#if( xcoretraceconfigSYSVIEW == 1 )
#include "SEGGER_SYSVIEW_FreeRTOS.h"

#define traceSTART() 						{ SEGGER_SYSVIEW_Conf(); SEGGER_SYSVIEW_Start(); }
#define traceEND()	 						{ SEGGER_SYSVIEW_Stop(); }

#define tracePRINTF_LOG( ... )				SEGGER_SYSVIEW_PrintfTarget( __VA_ARGS__ )
#define tracePRINTF_WARNING( ... )			SEGGER_SYSVIEW_WarnfTarget( __VA_ARGS__ )
#define tracePRINTF_ERROR( ... )			SEGGER_SYSVIEW_ErrorfTarget( __VA_ARGS__ )

#define xtrace_LOG_LVL						SEGGER_SYSVIEW_LOG
#define xtrace_WARN_LVL						SEGGER_SYSVIEW_WARNING
#define xtrace_ERR_LVL						SEGGER_SYSVIEW_ERROR

#define trace_printf( LOG_LEVEL, FMT, ... )	SEGGER_SYSVIEW_PrintfTargetEx( FMT, LOG_LEVEL, ##__VA_ARGS__ )

#endif /* xcoretraceconfigSYSVIEW */

#if( xcoretraceconfigASCII == 1 )
#include "ascii_trace.h"

#define xtrace_LOG_LVL						0
#define xtrace_WARN_LVL						1
#define xtrace_ERR_LVL						2

#define trace_printf( LOG_LEVEL, FMT, ... )	tracePRINTF( FMT, LOG_LEVEL, ##__VA_ARGS__ )

#endif /* xcoretraceconfigASCII */

#ifndef trace_printf
	/* Log messages to trace utility */
	#define trace_printf( LOG_LEVEL, FMT, ... )
#endif

#ifndef xtrace_LOG_LVL
	/* Level of log messages */
	#define xtrace_LOG_LVL
#endif

#ifndef xtrace_WARN_LVL
	/* Level of log warnings */
	#define xtrace_WARN_LVL
#endif

#ifndef xtrace_ERR_LVL
	/* Level of log errors */
	#define xtrace_ERR_LVL
#endif

#if xcoretraceconfigENABLE_PLUS_IP_TRACES
#include "xcore_IP_trace.h"
#endif
/*-----------------------------------------------------------*/

#endif /* __XC__ */

#endif /* no errors */

#endif /* XCORE_TRACE_H_ */
