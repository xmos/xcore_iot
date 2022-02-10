// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_H_

#define appconfGPIO_RPC_PORT 0

#define appconfI2C_CTRL_ENABLED 0
#define appconfUSB_CTRL_ENABLED 1
#define appconf_CONTROL_SERVICER_COUNT 2
#define appconfDEVICE_CONTROL_I2C_PORT 1
#define appconfDEVICE_CONTROL_USB_PORT 2
#define appconfI2C_INTERRUPT_CORE 0

#define GPIO_TILE_NO    0
#define I2C_TILE_NO     0

#define I2C_CTRL_TILE_NO    0

/* Task Priorities */
#define appconfDEVICE_CONTROL_I2C_CLIENT_PRIORITY   (configMAX_PRIORITIES-1)
#define appconfDEVICE_CONTROL_USB_CLIENT_PRIORITY   (configMAX_PRIORITIES-1)
#define appconfGPIO_RPC_HOST_TASK_PRIORITY          (configMAX_PRIORITIES/2)
#define appconfSTARTUP_TASK_PRIORITY                (configMAX_PRIORITIES-1)
#define appconfUSB_DEVICE_CTRL_TASK_PRIORITY        (configMAX_PRIORITIES-1)
#define appconfI2C_TASK_PRIORITY                    (configMAX_PRIORITIES/2)

#endif /* APP_CONF_H_ */
