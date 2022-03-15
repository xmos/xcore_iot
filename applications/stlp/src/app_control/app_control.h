// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef APP_CONTROL_H_
#define APP_CONTROL_H_

#include "device_control.h"

#include "app_conf.h"

extern device_control_t *device_control_i2c_ctx;
extern device_control_t *device_control_usb_ctx;

control_ret_t app_control_servicer_register(device_control_servicer_t *ctx,
                                            const control_resid_t resources[],
                                            size_t num_resources);
int app_control_init(void);



/*
 * TODO: Change the type of each state pointer in the handler
 * functions below to be whatever ends up being needed.
 */

/***** Audio Pipeline Control *****/
#define APP_CONTROL_RESID_AP 1
void app_control_ap_handler(void *state, unsigned timeout);
void app_control_ap_servicer_register(void);


/***** AEC Control *****/
#define APP_CONTROL_RESID_AEC 2
void app_control_aec_handler(void *state, unsigned timeout);
void app_control_aec_servicer_register();


/***** Stage 1 Control *****/
#define APP_CONTROL_RESID_STAGE1 3
void app_control_stage1_handler(void *state, unsigned timeout);
void app_control_stage1_servicer_register();


/***** Stage 2 Control *****/
#define APP_CONTROL_RESID_STAGE2 4
void app_control_stage2_handler(void *state, unsigned timeout);
void app_control_stage2_servicer_register();




#endif /* APP_CONTROL_H_ */
