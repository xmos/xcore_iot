// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef SPI_CAMERA_H_
#define SPI_CAMERA_H_

/* Read buffer must be large enough for 16bit 96x96 image, plus 1 for burst command */
#define IMAGE_BUF_SIZE     ((96*96*2) + 1)

#include "rtos/drivers/spi/api/rtos_spi_master.h"
#include "rtos/drivers/i2c/api/rtos_i2c_master.h"

BaseType_t create_spi_camera_to_queue(rtos_spi_master_device_t *spi_dev,
                                      rtos_i2c_master_t *i2c_dev,
                                      UBaseType_t priority,
                                      QueueHandle_t q_output );

#endif /* SPI_CAMERA_H_ */
