// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef INTERTILE_DRIVER_H_
#define INTERTILE_DRIVER_H_

#include "soc.h"
#include "intertile_dev_ctrl.h"

#include "FreeRTOS.h"

/* Attribute for the intertile_isr_callback_map member of the intertile isr.
 * Required by xcc to calculate stack usage.
 */
#define INTERTILE_ISR_CALLBACK_ATTR __attribute__((fptrgroup("intertile_isr_callback")))

/* INTERTILE ISR callback function macros. For xcc this ensures they get added
 * to the intertile isr callback group so that stack usage for certain functions
 * can be calculated.
 */
#define INTERTILE_ISR_CALLBACK_FUNCTION_PROTO( xFunction, device, buf, len, status, xReturnBufferToDMA ) BaseType_t xFunction( soc_peripheral_t device, uint8_t *buf, int len, uint32_t status, BaseType_t* xReturnBufferToDMA )
#define INTERTILE_ISR_CALLBACK_FUNCTION( xFunction, device, buf, len, status, xReturnBufferToDMA ) INTERTILE_ISR_CALLBACK_ATTR BaseType_t xFunction( soc_peripheral_t device, uint8_t *buf, int len, uint32_t status, BaseType_t* xReturnBufferToDMA )

typedef BaseType_t (*intertile_isr_cb_t)(soc_peripheral_t device, uint8_t *buf, int len, uint32_t status, BaseType_t* xReturnBufferToDMA);

typedef enum {
    INTERTILE_CB_ID_0,
    INTERTILE_CB_ID_1,
    INTERTILE_CB_ID_2,
    INTERTILE_CB_ID_3,
    INTERTILE_CB_ID_4,
    INTERTILE_CB_ID_5,
    INTERTILE_CB_ID_6,
    INTERTILE_CB_ID_7,
} intertile_cb_id_t;

typedef struct intertile_cb_footer {
    intertile_cb_id_t cb_id;
} intertile_cb_footer_t;

/**
 * Initialize the intertile device
 *
 * \param[in]     device_id      ID of intertile device to use
 * \param[in]     rx_desc_count  Maximum number of rx descriptors
 * \param[in]     tx_desc_count  Maximum number of tx descriptors
 * \param[in]     app_data       User data field
 * \param[in]     isr_core       FreeRTOS core to handle interrupts from device
 *
 * \returns       Initialized intertile device
 */
soc_peripheral_t intertile_driver_init(
        int device_id,
        int rx_desc_count,
        int tx_desc_count,
        void *app_data,
        int isr_core);

/**
 * Initialize an intertile message header
 *
 * Messages sent with the intertile device will trigger an ISR
 * on the receiver tile.  This configured header will trigger the
 * callback ISR mapped to cb_id, if one has been registered.
 *
 * \param[in/out] cb_footer      Pointer to an intertile header
 * \param[in]     cb_id          Intertile ID to map to this header
 *
 * \returns       pdPASS on success
 *                pdFAIL otherwise
 */
BaseType_t intertile_driver_footer_init(
        intertile_cb_footer_t* cb_footer,
        intertile_cb_id_t cb_id);

/**
 * Register an intertile ISR callback
 *
 * Register a callback to handle receiving a message with cb_footer.
 *
 * \param[in]     dev            Intertile device to use
 * \param[in]     isr_cb         ISR callback
 * \param[in]     cb_footer      Pointer to a configured header
 *
 * \returns       pdPASS on success
 *                pdFAIL otherwise
 */
BaseType_t intertile_driver_register_callback(
        soc_peripheral_t dev,
        intertile_isr_cb_t isr_cb,
        intertile_cb_footer_t* cb_footer);

/**
 * Unregister an intertile ISR callback
 *
 * Unregister
 *
 * \param[in]     dev            Intertile device to use
 * \param[in]     cb_footer      Pointer to a configured header
 *
 * \returns       pdPASS on success
 *                pdFAIL otherwise
 */
BaseType_t intertile_driver_unregister_callback(
        soc_peripheral_t dev,
        intertile_cb_footer_t* cb_footer);

void intertile_driver_send_bytes(
        soc_peripheral_t dev,
        uint8_t *bytes,
        unsigned len,
        intertile_cb_footer_t* cb_footer);


#endif /* INTERTILE_DRIVER_H_ */
