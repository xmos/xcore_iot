// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef TRACEMACRO_H_
#define TRACEMACRO_H_

#ifndef __ASSEMBLER__

#ifdef __cplusplus
extern "C" {
#endif

int xscope_gettime( void );

#ifdef configNUM_CORES
#define asciitraceGET_CORE_ID()		rtos_core_id_get()
#else
#define asciitraceGET_CORE_ID()		({(int)0;})
#endif

#ifdef THIS_XCORE_TILE
#define asciitraceGET_TILE_ID()		({(int)THIS_XCORE_TILE;})
#else
#define asciitraceGET_TILE_ID()		get_local_tile_id()
#endif


void traceFreeRTOS_to_xscope(char* fmt, ...);

#define traceOUTPUT(...)										\
			do {												\
				uint32_t ulState = portDISABLE_INTERRUPTS();	\
				traceFreeRTOS_to_xscope(__VA_ARGS__);			\
				portRESTORE_INTERRUPTS(ulState);				\
			} while(0)

#define traceTASK_SWITCHED_OUT()    traceOUTPUT( "%d:%d:%d:OUT:%s\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), pxCurrentTCB->pcTaskName )
#define traceTASK_SWITCHED_IN()     traceOUTPUT( "%d:%d:%d:IN:%s\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), pxCurrentTCB->pcTaskName )

#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#endif /* TRACEMACRO_H_ */
