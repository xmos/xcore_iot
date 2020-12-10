// Copyright (c) 2020, XMOS Ltd, All rights reserved

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



/*
 * Formatting is as follows:
 * traceOUTPUT( "%d:%d:%d:%d,...\n", calling tile, calling core, traceID, runtime, [ additional args ] );
 * Additional arguments shall be comma separated
 * Note: Spaces in format string are ignored and are not output.
 *
 * Ex:
 * traceOUTPUT( "%d:%d:%d:%d,%d,%s\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), 0, 30, "hello world" );
 */

typedef enum
{
	etraceUSER_MSG = 0,
	etraceSTART,
	etraceEND,
	etraceMOVED_TASK_TO_READY_STATE,
	etracePOST_MOVED_TASK_TO_READY_STATE,
	etraceCREATE_MUTEX,
	etraceCREATE_MUTEX_FAILED,
	etraceGIVE_MUTEX_RECURSIVE,
	etraceGIVE_MUTEX_RECURSIVE_FAILED,
	etraceTAKE_MUTEX_RECURSIVE,
	etraceTAKE_MUTEX_RECURSIVE_FAILED,
	etraceCREATE_COUNTING_SEMAPHORE,
	etraceCREATE_COUNTING_SEMAPHORE_FAILED,
	etraceQUEUE_CREATE,
	etraceQUEUE_CREATE_FAILED,
	etraceQUEUE_SEND,
	etraceQUEUE_SEND_FAILED,
	etraceQUEUE_RECEIVE,
	etraceQUEUE_PEEK,
	etraceQUEUE_PEEK_FAILED,
	etraceQUEUE_PEEK_FROM_ISR,
	etraceQUEUE_RECEIVE_FAILED,
	etraceQUEUE_SEND_FROM_ISR,
	etraceQUEUE_SEND_FROM_ISR_FAILED,
	etraceQUEUE_RECEIVE_FROM_ISR,
	etraceQUEUE_RECEIVE_FROM_ISR_FAILED,
	etraceQUEUE_PEEK_FROM_ISR_FAILED,
	etraceQUEUE_DELETE,
	etraceQUEUE_REGISTRY_ADD,
	etraceBLOCKING_ON_QUEUE_SEND,
	etraceBLOCKING_ON_QUEUE_PEEK,
	etraceBLOCKING_ON_QUEUE_RECEIVE,
	etraceTASK_CREATE,
	etraceTASK_CREATE_FAILED,
	etraceTASK_DELETE,
	etraceTASK_DELAY_UNTIL,
	etraceTASK_DELAY,
	etraceTASK_PRIORITY_SET,
	etraceTASK_SUSPEND,
	etraceTASK_RESUME,
	etraceTASK_RESUME_FROM_ISR,
	etraceTASK_INCREMENT_TICK,
	etraceTASK_NOTIFY_TAKE_BLOCK,
	etraceTASK_NOTIFY_TAKE,
	etraceTASK_NOTIFY_WAIT_BLOCK,
	etraceTASK_NOTIFY_WAIT,
	etraceTASK_NOTIFY,
	etraceTASK_NOTIFY_FROM_ISR,
	etraceTASK_NOTIFY_GIVE_FROM_ISR,
	etraceTASK_PRIORITY_DISINHERIT,
	etraceTASK_PRIORITY_INHERIT,
	etraceTASK_SWITCHED_OUT,
	etraceTASK_SWITCHED_IN,
	etraceLOW_POWER_IDLE_BEGIN,
	etraceLOW_POWER_IDLE_END,
	etraceTIMER_CREATE,
	etraceTIMER_CREATE_FAILED,
	etraceTIMER_COMMAND_SEND,
	etraceTIMER_EXPIRED,
	etraceTIMER_COMMAND_RECEIVED,
	etraceMALLOC,
	etraceFREE,
	etraceEVENT_GROUP_CREATE,
	etraceEVENT_GROUP_CREATE_FAILED,
	etraceEVENT_GROUP_SYNC_BLOCK,
	etraceEVENT_GROUP_SYNC_END,
	etraceEVENT_GROUP_WAIT_BITS_BLOCK,
	etraceEVENT_GROUP_WAIT_BITS_END,
	etraceEVENT_GROUP_CLEAR_BITS,
	etraceEVENT_GROUP_CLEAR_BITS_FROM_ISR,
	etraceEVENT_GROUP_SET_BITS,
	etraceEVENT_GROUP_SET_BITS_FROM_ISR,
	etraceEVENT_GROUP_DELETE,
	etracePEND_FUNC_CALL,
	etracePEND_FUNC_CALL_FROM_ISR,
	etraceSTREAM_BUFFER_CREATE_FAILED,
	etraceSTREAM_BUFFER_CREATE_STATIC_FAILED,
	etraceSTREAM_BUFFER_CREATE,
	etraceSTREAM_BUFFER_DELETE,
	etraceSTREAM_BUFFER_RESET,
	etraceBLOCKING_ON_STREAM_BUFFER_SEND,
	etraceSTREAM_BUFFER_SEND,
	etraceSTREAM_BUFFER_SEND_FAILED,
	etraceSTREAM_BUFFER_SEND_FROM_ISR,
	etraceBLOCKING_ON_STREAM_BUFFER_RECEIVE,
	etraceSTREAM_BUFFER_RECEIVE,
	etraceSTREAM_BUFFER_RECEIVE_FAILED,
	etraceSTREAM_BUFFER_RECEIVE_FROM_ISR,
	etraceINCREASE_TICK_COUNT,
	etraceTOTAL_TRACE_COUNT
} eTraceMap_t;


#define tracePRINTF( FMT, LOG_LEVEL, ... )				traceOUTPUT( "%d:%d:%d:%d,%d," FMT "\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceUSER_MSG, LOG_LEVEL, ##__VA_ARGS__ )

#define traceSTART()									traceOUTPUT( "%d:%d:%d:%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceSTART )
#define traceEND()										traceOUTPUT( "%d:%d:%d:%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceEND )
#define traceMOVED_TASK_TO_READY_STATE( pxTCB )			traceOUTPUT( "%d:%d:%d:%d,%s\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceMOVED_TASK_TO_READY_STATE, pxTCB->pcTaskName )
#define tracePOST_MOVED_TASK_TO_READY_STATE( pxTCB )	traceOUTPUT( "%d:%d:%d:%d,%s\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etracePOST_MOVED_TASK_TO_READY_STATE, pxTCB->pcTaskName )
#define traceCREATE_MUTEX( pxNewQueue )					traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceCREATE_MUTEX, pxNewQueue->uxQueueNumber )
#define traceCREATE_MUTEX_FAILED()						traceOUTPUT( "%d:%d:%d:%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceCREATE_MUTEX_FAILED )
#define traceGIVE_MUTEX_RECURSIVE( pxMutex )			traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceGIVE_MUTEX_RECURSIVE, pxMutex->uxQueueNumber )
#define traceGIVE_MUTEX_RECURSIVE_FAILED( pxMutex )		traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceGIVE_MUTEX_RECURSIVE_FAILED, pxMutex->uxQueueNumber )
#define traceTAKE_MUTEX_RECURSIVE( pxMutex )			traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTAKE_MUTEX_RECURSIVE, pxMutex->uxQueueNumber )
#define traceTAKE_MUTEX_RECURSIVE_FAILED( pxMutex )		traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTAKE_MUTEX_RECURSIVE_FAILED, pxMutex->uxQueueNumber )
#define traceCREATE_COUNTING_SEMAPHORE()				traceOUTPUT( "%d:%d:%d:%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceCREATE_COUNTING_SEMAPHORE )
#define traceCREATE_COUNTING_SEMAPHORE_FAILED()			traceOUTPUT( "%d:%d:%d:%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceCREATE_COUNTING_SEMAPHORE_FAILED )
#define traceQUEUE_CREATE( pxNewQueue )					traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceQUEUE_CREATE, pxNewQueue->uxQueueNumber )
#define traceQUEUE_CREATE_FAILED( ucQueueType )			traceOUTPUT( "%d:%d:%d:%d,%u\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceQUEUE_CREATE_FAILED, ucQueueType )
#define traceQUEUE_SEND( pxQueue )						traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceQUEUE_SEND, pxQueue->uxQueueNumber )
#define traceQUEUE_SEND_FAILED( pxQueue )				traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceQUEUE_SEND_FAILED, pxQueue->uxQueueNumber )
#define traceQUEUE_RECEIVE( pxQueue )					traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceQUEUE_RECEIVE, pxQueue->uxQueueNumber )
#define traceQUEUE_PEEK( pxQueue )						traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceQUEUE_PEEK, pxQueue->uxQueueNumber )
#define traceQUEUE_PEEK_FAILED( pxQueue )				traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceQUEUE_PEEK_FAILED, pxQueue->uxQueueNumber )
#define traceQUEUE_PEEK_FROM_ISR( pxQueue )				traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceQUEUE_PEEK_FROM_ISR, pxQueue->uxQueueNumber )
#define traceQUEUE_RECEIVE_FAILED( pxQueue )			traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceQUEUE_RECEIVE_FAILED, pxQueue->uxQueueNumber )
#define traceQUEUE_SEND_FROM_ISR( pxQueue )				traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceQUEUE_SEND_FROM_ISR, pxQueue->uxQueueNumber )
#define traceQUEUE_SEND_FROM_ISR_FAILED( pxQueue )		traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceQUEUE_SEND_FROM_ISR_FAILED, pxQueue->uxQueueNumber )
#define traceQUEUE_RECEIVE_FROM_ISR( pxQueue )			traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceQUEUE_RECEIVE_FROM_ISR, pxQueue->uxQueueNumber )
#define traceQUEUE_RECEIVE_FROM_ISR_FAILED( pxQueue )	traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceQUEUE_RECEIVE_FROM_ISR_FAILED, pxQueue->uxQueueNumber )
#define traceQUEUE_PEEK_FROM_ISR_FAILED( pxQueue )		traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceQUEUE_PEEK_FROM_ISR_FAILED, pxQueue->uxQueueNumber )
#define traceQUEUE_DELETE( pxQueue )					traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceQUEUE_DELETE, pxQueue->uxQueueNumber )
#define traceQUEUE_REGISTRY_ADD(xQueue, pcQueueName)	traceOUTPUT( "%d:%d:%d:%d,%d,%s\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceQUEUE_REGISTRY_ADD, xQueue->uxQueueNumber, pcQueueName )
#define traceBLOCKING_ON_QUEUE_SEND( pxQueue )			traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceBLOCKING_ON_QUEUE_SEND, pxQueue->uxQueueNumber )
#define traceBLOCKING_ON_QUEUE_PEEK( pxQueue )			traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceBLOCKING_ON_QUEUE_PEEK, pxQueue->uxQueueNumber )
#define traceBLOCKING_ON_QUEUE_RECEIVE( pxQueue )		traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceBLOCKING_ON_QUEUE_RECEIVE, pxQueue->uxQueueNumber )
#define traceLOW_POWER_IDLE_BEGIN()						traceOUTPUT( "%d:%d:%d:%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceLOW_POWER_IDLE_BEGIN )
#define traceLOW_POWER_IDLE_END()						traceOUTPUT( "%d:%d:%d:%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceLOW_POWER_IDLE_END )
#define traceMALLOC( pvAddress, uiSize )				traceOUTPUT( "%d:%d:%d:%d,%p,%u\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceMALLOC, pvAddress, uiSize )
#define traceFREE( pvAddress, uiSize )					traceOUTPUT( "%d:%d:%d:%d,%p,%u\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceFREE, pvAddress, uiSize )
#define traceTASK_CREATE( pxNewTCB )					traceOUTPUT( "%d:%d:%d:%d,%s\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTASK_CREATE, pxNewTCB->pcTaskName )
#define traceTASK_CREATE_FAILED()						traceOUTPUT( "%d:%d:%d:%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTASK_CREATE_FAILED )
#define traceTASK_DELETE( pxTaskToDelete )				traceOUTPUT( "%d:%d:%d:%d,%s\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTASK_DELETE, pxTaskToDelete->pcTaskName )
#define traceTASK_DELAY_UNTIL( x )						traceOUTPUT( "%d:%d:%d:%d,%d,%s\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTASK_DELAY_UNTIL, x, pxCurrentTCB->pcTaskName )
#define traceTASK_DELAY()								traceOUTPUT( "%d:%d:%d:%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTASK_DELAY )
#define traceTASK_PRIORITY_SET( pxTask, uxNewPriority )	traceOUTPUT( "%d:%d:%d:%d,%s,%u,%u\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTASK_PRIORITY_SET, pxTask->pcTaskName, pxTask->uxPriority, uxNewPriority )
#define traceTASK_SUSPEND( pxTaskToSuspend )			traceOUTPUT( "%d:%d:%d:%d,%s\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTASK_SUSPEND, pxTaskToSuspend->pcTaskName )
#define traceTASK_RESUME( pxTaskToResume )				traceOUTPUT( "%d:%d:%d:%d,%s\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTASK_RESUME, pxTaskToResume->pcTaskName )
#define traceTASK_RESUME_FROM_ISR( pxTaskToResume )		traceOUTPUT( "%d:%d:%d:%d,%s\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTASK_RESUME_FROM_ISR, pxTaskToResume->pcTaskName )
#define traceTASK_INCREMENT_TICK( xTickCount )			traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTASK_INCREMENT_TICK, xTickCount )
#define traceTASK_NOTIFY_TAKE_BLOCK()					traceOUTPUT( "%d:%d:%d:%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTASK_NOTIFY_TAKE_BLOCK )
#define traceTASK_NOTIFY_TAKE()							traceOUTPUT( "%d:%d:%d:%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTASK_NOTIFY_TAKE )
#define traceTASK_NOTIFY_WAIT_BLOCK()					traceOUTPUT( "%d:%d:%d:%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTASK_NOTIFY_WAIT_BLOCK )
#define traceTASK_NOTIFY_WAIT()							traceOUTPUT( "%d:%d:%d:%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTASK_NOTIFY_WAIT )
#define traceTASK_NOTIFY()								traceOUTPUT( "%d:%d:%d:%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTASK_NOTIFY )
#define traceTASK_NOTIFY_FROM_ISR()						traceOUTPUT( "%d:%d:%d:%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTASK_NOTIFY_FROM_ISR )
#define traceTASK_NOTIFY_GIVE_FROM_ISR()				traceOUTPUT( "%d:%d:%d:%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTASK_NOTIFY_GIVE_FROM_ISR )
#define traceTASK_SWITCHED_OUT()						traceOUTPUT( "%d:%d:%d:%d,%s\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTASK_SWITCHED_OUT, pxCurrentTCB->pcTaskName )
#define traceTASK_SWITCHED_IN()							traceOUTPUT( "%d:%d:%d:%d,%s\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTASK_SWITCHED_IN, pxCurrentTCB->pcTaskName )
#define traceTASK_PRIORITY_DISINHERIT( pxTCBOfMutexHolder, uxOriginalPriority )						\
														traceOUTPUT( "%d:%d:%d:%d,%s,%u\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTASK_PRIORITY_DISINHERIT, pxTCBOfMutexHolder->pcTaskName, uxOriginalPriority )
#define traceTASK_PRIORITY_INHERIT( pxTCBOfMutexHolder, uxInheritedPriority )						\
														traceOUTPUT( "%d:%d:%d:%d,%s,%u\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTASK_PRIORITY_INHERIT, pxTCBOfMutexHolder->pcTaskName, uxInheritedPriority )
#define traceTIMER_CREATE( pxNewTimer )					traceOUTPUT( "%d:%d:%d:%d,%s\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTIMER_CREATE, pxNewTimer->pcTimerName )
#define traceTIMER_CREATE_FAILED()						traceOUTPUT( "%d:%d:%d:%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTIMER_CREATE_FAILED )
#define traceTIMER_COMMAND_SEND( xTimer, xMessageID, xMessageValueValue, xReturn )					\
														traceOUTPUT( "%d:%d:%d:%d,%s,%d,%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTIMER_COMMAND_SEND, xTimer->pcTimerName, xMessageID, xMessageValueValue, xReturn )
#define traceTIMER_EXPIRED( pxTimer )					traceOUTPUT( "%d:%d:%d:%d,%s\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTIMER_EXPIRED, pxTimer->pcTimerName )
#define traceTIMER_COMMAND_RECEIVED( pxTimer, xMessageID, xMessageValue )							\
														traceOUTPUT( "%d:%d:%d:%d,%s,%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceTIMER_COMMAND_RECEIVED, pxTimer->pcTimerName, xMessageID, xMessageValue )
#define traceEVENT_GROUP_CREATE( xEventGroup )			traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceEVENT_GROUP_CREATE, xEventGroup->uxEventGroupNumber )
#define traceEVENT_GROUP_CREATE_FAILED()				traceOUTPUT( "%d:%d:%d:%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceEVENT_GROUP_CREATE_FAILED )
#define traceEVENT_GROUP_SYNC_BLOCK( xEventGroup, uxBitsToSet, uxBitsToWaitFor )					\
														traceOUTPUT( "%d:%d:%d:%d,%u,%u,%u\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceEVENT_GROUP_SYNC_BLOCK, xEventGroup->uxEventGroupNumber, uxBitsToSet, uxBitsToWaitFor )
#define traceEVENT_GROUP_SYNC_END( xEventGroup, uxBitsToSet, uxBitsToWaitFor, xTimeoutOccurred )	\
														traceOUTPUT( "%d:%d:%d:%d,%u,%u,%u,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceEVENT_GROUP_SYNC_END, xEventGroup->uxEventGroupNumber, uxBitsToSet, uxBitsToWaitFor, xTimeoutOccurred )
#define traceEVENT_GROUP_WAIT_BITS_BLOCK( xEventGroup, uxBitsToWaitFor )							\
														traceOUTPUT( "%d:%d:%d:%d,%d,%u\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceEVENT_GROUP_WAIT_BITS_BLOCK, xEventGroup->uxEventGroupNumber, uxBitsToWaitFor )
#define traceEVENT_GROUP_WAIT_BITS_END( xEventGroup, uxBitsToWaitFor, xTimeoutOccurred )			\
														traceOUTPUT( "%d:%d:%d:%d,%d,%u,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceEVENT_GROUP_WAIT_BITS_END, xEventGroup->uxEventGroupNumber, uxBitsToWaitFor, xTimeoutOccurred )
#define traceEVENT_GROUP_CLEAR_BITS( xEventGroup, uxBitsToClear )									\
														traceOUTPUT( "%d:%d:%d:%d,%d,%u\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceEVENT_GROUP_CLEAR_BITS, xEventGroup->uxEventGroupNumber, uxBitsToClear )
#define traceEVENT_GROUP_CLEAR_BITS_FROM_ISR( xEventGroup, uxBitsToClear )							\
														traceOUTPUT( "%d:%d:%d:%d,%d,%u\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceEVENT_GROUP_CLEAR_BITS_FROM_ISR, xEventGroup->uxEventGroupNumber, uxBitsToClear )
#define traceEVENT_GROUP_SET_BITS( xEventGroup, uxBitsToSet )										\
														traceOUTPUT( "%d:%d:%d:%d,%d,%u\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceEVENT_GROUP_SET_BITS, xEventGroup->uxEventGroupNumber, uxBitsToSet )
#define traceEVENT_GROUP_SET_BITS_FROM_ISR( xEventGroup, uxBitsToSet )								\
														traceOUTPUT( "%d:%d:%d:%d,%d,%u\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceEVENT_GROUP_SET_BITS_FROM_ISR, xEventGroup->uxEventGroupNumber, uxBitsToSet )
#define traceEVENT_GROUP_DELETE( xEventGroup )														\
														traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceEVENT_GROUP_DELETE, xEventGroup->uxEventGroupNumber )
#define tracePEND_FUNC_CALL(xFunctionToPend, pvParameter1, ulParameter2, ret )						\
        												traceOUTPUT( "%d:%d:%d:%d,%p,%p,%u,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etracePEND_FUNC_CALL, xFunctionToPend, pvParameter1, ulParameter2, ret )
#define tracePEND_FUNC_CALL_FROM_ISR(xFunctionToPend, pvParameter1, ulParameter2, ret)				\
														traceOUTPUT( "%d:%d:%d:%d,%p,%p,%u,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etracePEND_FUNC_CALL_FROM_ISR, xFunctionToPend, pvParameter1, ulParameter2, ret )
#define traceSTREAM_BUFFER_CREATE_FAILED( xIsMessageBuffer )										\
        												traceOUTPUT( "%d:%d:%d:%d,%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceSTREAM_BUFFER_CREATE_FAILED, xIsMessageBuffer )
#define traceSTREAM_BUFFER_CREATE_STATIC_FAILED( xReturn, xIsMessageBuffer )						\
														traceOUTPUT( "%d:%d:%d:%d,%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceSTREAM_BUFFER_CREATE_STATIC_FAILED, xReturn, xIsMessageBuffer )
#define traceSTREAM_BUFFER_CREATE( pxStreamBuffer, xIsMessageBuffer )								\
														traceOUTPUT( "%d:%d:%d:%d,%p,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceSTREAM_BUFFER_CREATE, pxStreamBuffer, xIsMessageBuffer )
#define traceSTREAM_BUFFER_DELETE( xStreamBuffer )		traceOUTPUT( "%d:%d:%d:%d,%u\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceSTREAM_BUFFER_DELETE, xStreamBuffer->uxStreamBufferNumber )
#define traceSTREAM_BUFFER_RESET( xStreamBuffer )		traceOUTPUT( "%d:%d:%d:%d,%u\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceSTREAM_BUFFER_RESET, xStreamBuffer->uxStreamBufferNumber )
#define traceBLOCKING_ON_STREAM_BUFFER_SEND( xStreamBuffer )										\
        												traceOUTPUT( "%d:%d:%d:%d,%u\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceBLOCKING_ON_STREAM_BUFFER_SEND, xStreamBuffer->uxStreamBufferNumber )
#define traceSTREAM_BUFFER_SEND( xStreamBuffer, xBytesSent )										\
														traceOUTPUT( "%d:%d:%d:%d,%u,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceSTREAM_BUFFER_SEND, xStreamBuffer->uxStreamBufferNumber, xBytesSent )
#define traceSTREAM_BUFFER_SEND_FAILED( xStreamBuffer )	traceOUTPUT( "%d:%d:%d:%d,%u\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceSTREAM_BUFFER_SEND_FAILED, xStreamBuffer->uxStreamBufferNumber )
#define traceSTREAM_BUFFER_SEND_FROM_ISR( xStreamBuffer, xBytesSent )								\
														traceOUTPUT( "%d:%d:%d:%d,%u,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceSTREAM_BUFFER_SEND_FROM_ISR, xStreamBuffer->uxStreamBufferNumber, xBytesSent )
#define traceBLOCKING_ON_STREAM_BUFFER_RECEIVE( xStreamBuffer )										\
														traceOUTPUT( "%d:%d:%d:%d,%u\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceBLOCKING_ON_STREAM_BUFFER_RECEIVE, xStreamBuffer->uxStreamBufferNumber )
#define traceSTREAM_BUFFER_RECEIVE( xStreamBuffer, xReceivedLength )								\
														traceOUTPUT( "%d:%d:%d:%d,%u,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceSTREAM_BUFFER_RECEIVE, xStreamBuffer->uxStreamBufferNumber, xReceivedLength )
#define traceSTREAM_BUFFER_RECEIVE_FAILED( xStreamBuffer )											\
														traceOUTPUT( "%d:%d:%d:%d,%u\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceSTREAM_BUFFER_RECEIVE_FAILED, xStreamBuffer->uxStreamBufferNumber )
#define traceSTREAM_BUFFER_RECEIVE_FROM_ISR( xStreamBuffer, xReceivedLength )						\
														traceOUTPUT( "%d:%d:%d:%d,%u,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceSTREAM_BUFFER_RECEIVE_FROM_ISR, xStreamBuffer->uxStreamBufferNumber, xReceivedLength )
#define traceINCREASE_TICK_COUNT( x )					traceOUTPUT( "%d:%d:%d:%d,%d\n", asciitraceGET_TILE_ID(), asciitraceGET_CORE_ID(), xscope_gettime(), etraceINCREASE_TICK_COUNT, x )


#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#endif /* TRACEMACRO_H_ */
