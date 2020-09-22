// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef AI_DRIVER_H_
#define AI_DRIVER_H_

#include "soc.h"

#include "FreeRTOS.h"

/* Attribute for the ai_isr_callback_map member of the ai isr.
 * Required by xcc to calculate stack usage.
 */
#define AI_ISR_CALLBACK_ATTR __attribute__((fptrgroup("ai_isr_callback")))

/* AI ISR callback function macros. For xcc this ensures they get added
 * to the intertile isr callback group so that stack usage for certain functions
 * can be calculated.
 */
#define AI_ISR_CALLBACK_FUNCTION_PROTO( xFunction, buf, len, status, args, xYieldRequired ) void xFunction( uint8_t *buf, int len, uint32_t status, void *args, BaseType_t *xYieldRequired )
#define AI_ISR_CALLBACK_FUNCTION( xFunction, buf, len, status, args, xYieldRequired ) AI_ISR_CALLBACK_ATTR void xFunction( uint8_t *buf, int len, uint32_t status, void *args, BaseType_t *xYieldRequired )

typedef void (*ai_isr_cb_t)(uint8_t *buf, int len, uint32_t status, void *args, BaseType_t *xYieldRequired);

/* Initialize driver*/
soc_peripheral_t ai_driver_init(
        int isr_core,
		ai_isr_cb_t isr_cb,
        void* args);

/* Setup */
int32_t ai_setup( soc_peripheral_t dev );

/* Trigger an inference */
void ai_invoke( soc_peripheral_t dev );

/* Populate input tensor */
int32_t ai_set_input_tensor( soc_peripheral_t dev, uint8_t* buf, int32_t num_bytes );

#endif /* AI_DRIVER_H_ */
