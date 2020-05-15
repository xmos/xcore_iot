/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*            (c) 1995 - 2019 SEGGER Microcontroller GmbH             *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
*       SEGGER SystemView * Real-time application analysis           *
*                                                                    *
**********************************************************************
*                                                                    *
* All rights reserved.                                               *
*                                                                    *
* SEGGER strongly recommends to not make any changes                 *
* to or modify the source code of this software in order to stay     *
* compatible with the RTT protocol and J-Link.                       *
*                                                                    *
* Redistribution and use in source and binary forms, with or         *
* without modification, are permitted provided that the following    *
* conditions are met:                                                *
*                                                                    *
* o Redistributions of source code must retain the above copyright   *
*   notice, this list of conditions and the following disclaimer.    *
*                                                                    *
* o Redistributions in binary form must reproduce the above          *
*   copyright notice, this list of conditions and the following      *
*   disclaimer in the documentation and/or other materials provided  *
*   with the distribution.                                           *
*                                                                    *
* o Neither the name of SEGGER Microcontroller GmbH         *
*   nor the names of its contributors may be used to endorse or      *
*   promote products derived from this software without specific     *
*   prior written permission.                                        *
*                                                                    *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             *
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,        *
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           *
* DISCLAIMED. IN NO EVENT SHALL SEGGER Microcontroller BE LIABLE FOR *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR           *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT  *
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;    *
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE  *
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH   *
* DAMAGE.                                                            *
*                                                                    *
**********************************************************************
*                                                                    *
*       SystemView version: V2.52h                                    *
*                                                                    *
**********************************************************************
-------------------------- END-OF-HEADER -----------------------------

File    : SEGGER_SYSVIEW_Config_FreeRTOS.c
Purpose : Sample setup configuration of SystemView with FreeRTOS
		  on XCORE.
Revision: $Rev: 7745 $
*/

#include "FreeRTOS.h"

#if xcoretraceconfigSYSVIEW == 1

#include "SEGGER_SYSVIEW.h"

int xscope_gettime( void );
extern const SEGGER_SYSVIEW_OS_API SYSVIEW_X_OS_TraceAPI;


/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
// The application name to be displayed in SystemViewer
#ifdef configNUM_CORES
#define SYSVIEW_APP_NAME        "FreeRTOS SMP Demo Application"
#else
#define SYSVIEW_APP_NAME        "FreeRTOS Demo Application"
#endif

// The target device name
#define SYSVIEW_DEVICE_NAME     "XCORE-XS2"

// Frequency of the timestamp. Must match SEGGER_SYSVIEW_GET_TIMESTAMP in SEGGER_SYSVIEW_Conf.h
#define SYSVIEW_TIMESTAMP_FREQ  configCPU_CLOCK_HZ

// System Frequency. SystemcoreClock is used in most CMSIS compatible projects.
#define SYSVIEW_CPU_FREQ        configCPU_CLOCK_HZ

// The lowest RAM address used for IDs (pointers)
#define SYSVIEW_RAM_BASE        (0x10000000)	// Default value, but not used for XCORE trace

/********************************************************************* 
*
*       _cbSendSystemDesc()
*
*  Function description
*    Sends SystemView description strings.
*/
static seggerMODULE_DESC_FUNCTION( _cbSendSystemDesc ) {
  SEGGER_SYSVIEW_SendSysDesc("N="SYSVIEW_APP_NAME",D="SYSVIEW_DEVICE_NAME",O=FreeRTOS");
  SEGGER_SYSVIEW_SendSysDesc("I#01=SysTick");
#ifdef configNUM_CORES
  SEGGER_SYSVIEW_SendSysDesc("I#02=InterCore");
#endif
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/
void SEGGER_SYSVIEW_Conf(void) {
  SEGGER_SYSVIEW_Init(SYSVIEW_TIMESTAMP_FREQ, SYSVIEW_CPU_FREQ, 
                      &SYSVIEW_X_OS_TraceAPI, _cbSendSystemDesc);
  SEGGER_SYSVIEW_SetRAMBase(SYSVIEW_RAM_BASE);
}


/*********************************************************************
*
*       SEGGER_SYSVIEW_X_GetTimestamp()
*
* Function description
*   Returns the current timestamp in ticks
*
* Return value
*   The current timestamp.
*
*/
seggerGET_TIME_FUNCTION( SEGGER_SYSVIEW_X_GetTimestamp ){
	return xscope_gettime();
}

/*********************************************************************
*
*       SEGGER_SYSVIEW_X_GetInterruptId()
*
*  Function description
*    Return the priority of the currently active interrupt.
*/
U32 SEGGER_SYSVIEW_X_GetInterruptId(void) {
  U32 IntId = 0;	// No longer called for XCORE.  Interrupt ID now passed via trace call
  return IntId;
}

#endif /* xcoretraceconfigSYSVIEW == 1 */

/*************************** End of file ****************************/
