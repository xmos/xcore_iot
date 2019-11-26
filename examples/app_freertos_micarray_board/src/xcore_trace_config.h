/*
 * xcore_trace_config.h
 *
 *  Created on: Sep 13, 2019
 *      Author: jmccarthy
 */


#ifndef XCORE_TRACE_CONFIG_H_
#define XCORE_TRACE_CONFIG_H_

/* Enable trace system */
#define xcoretraceconfigSYSVIEW 					0
#define xcoretraceconfigASCII 						0

/* Setup xScope trace Macros */
#define xcoretraceconfigXSCOPE_TRACE_BUFFER         200
#define xcoretraceconfigXSCOPE_TRACE_RAW_BYTES      0

/* Include Plus tracing */
#define xcoretraceconfigENABLE_PLUS_IP_TRACES       0

#endif /* XCORE_TRACE_CONFIG_H_ */
