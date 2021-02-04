// Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1

#ifndef SPI_CAMERA_H_
#define SPI_CAMERA_H_

/* Read buffer must be large enough for 16bit 96x96 image, plus 1 for burst command */
#define IMAGE_BUF_SIZE     ((96*96*2) + 1)

BaseType_t create_spi_camera_to_queue( UBaseType_t priority, QueueHandle_t q_output );

#endif /* SPI_CAMERA_H_ */
