// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef APP_CONF_H_
#define APP_CONF_H_

// Port configuration

#define appconfI2C_MASTER_RPC_PORT 9
#define appconfMIC_ARRAY_RPC_PORT 10
#define appconfI2S_RPC_PORT 11
#define appconfGPIO_RPC_PORT 12
#define appconfSPI_RPC_PORT 13
#define appconfQSPI_RPC_PORT 14

// Clock configuration

#define appconfAUDIO_CLOCK_FREQUENCY 24576000
#define appconfPDM_CLOCK_FREQUENCY 3072000

// RPC configuration

#ifndef appconfI2C_RPC_ENABLED
#define appconfI2C_RPC_ENABLED 1
#endif

#ifndef appconfMIC_ARRAY_RPC_ENABLED
#define appconfMIC_ARRAY_RPC_ENABLED 1
#endif

#ifndef appconfI2S_RPC_ENABLED
#define appconfI2S_RPC_ENABLED 1
#endif

#ifndef appconfGPIO_RPC_ENABLED
#define appconfGPIO_RPC_ENABLED 1
#endif

#ifndef appconfSPI_RPC_ENABLED
#define appconfSPI_RPC_ENABLED 1
#endif

#ifndef appconfQSPI_FLASH_RPC_ENABLED
#define appconfQSPI_FLASH_RPC_ENABLED 1
#endif

#ifndef appconfI2S_ADC_ENABLED
#define appconfI2S_ADC_ENABLED 1
#endif

// Test configuration

#ifndef appconfINTERTILE_TEST
#define appconfINTERTILE_TEST 0
#endif

#ifndef appconfRPC_TEST
#define appconfRPC_TEST 0
#endif

#ifndef appconfGPIO_TEST
#define appconfGPIO_TEST 1
#endif

#ifndef appconfWIFI_TEST
#define appconfWIFI_TEST 1
#endif

#ifndef appconfQSPI_TEST
#define appconfQSPI_TEST 0
#endif

#ifndef appconfSW_MEM_TEST
#define appconfSW_MEM_TEST 1
#endif

// Task Priorities
#define appconfSTARTUP_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define appconfI2C_MASTER_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES / 2)
#define appconfMIC_ARRAY_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES / 2)
#define appconfI2S_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES / 2)
#define appconfGPIO_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES / 2)
#define appconfSPI_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES / 2)
#define appconfQSPI_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES / 2 - 1)

#endif /* APP_CONF_H_ */