// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef OV2640_H_
#define OV2640_H_

#include <stdint.h>

#define OV2640_I2C_ADDR                 0x30
#define OV2640_CHIPID_HIGH 	            0x0A
#define OV2640_CHIPID_LOW 	            0x0B

#define ARDUCAM_TEST_REG                0x00
#define ARDUCAM_FRAMES_REG              0x01
#define ARDUCAM_MODE_REG                0x02
    #define ARDUCAM_MODE_MCU2LCD            0x00
    #define ARDUCAM_MODE_CAM2LCD            0x01
    #define ARDUCAM_MODE_LCD2MCU            0x02
#define ARDUCAM_TIMING_REG              0x03
    #define ARDUCAM_TIMING_FIFO_MODE_MASK   0x10
#define ARDUCAM_FIFO      		        0x04
    #define ARDUCAM_FIFO_CLEAR_MASK    		0x01
    #define ARDUCAM_FIFO_START_MASK    		0x02
    #define ARDUCAM_FIFO_RDPTR_RST_MASK     0x10
    #define ARDUCAM_FIFO_WRPTR_RST_MASK     0x20
#define ARDUCAM_GPIO_DIR_REG            0x05
    #define ARDUCAM_GPIO_DIR_RESET_MASK	    0x01
    #define ARDUCAM_GPIO_DIR_PWDN_MASK		0x02
    #define ARDUCAM_GPIO_DIR_PWREN_MASK		0x04
#define ARDUCAM_GPIO			        0x06
    #define ARDUCAM_GPIO_RESET_MASK	        0x01
    #define ARDUCAM_GPIO_PWDN_MASK          0x02
    #define ARDUCAM_GPIO_PWREN_MASK         0x04
#define ARDUCAM_REV       		        0x40
    #define ARDUCAM_REV_VER_LOW_MASK        0x3F
    #define ARDUCAM_REV_VER_HIGH_MASK       0xC0
#define ARDUCAM_TRIG      		        0x41
    #define ARDUCAM_TRIG_VSYNC_MASK         0x01
    #define ARDUCAM_TRIG_CAP_DONE_MASK      0x08
#define ARDUCAM_FIFO_SIZE1				0x42
#define ARDUCAM_FIFO_SIZE2				0x43
#define ARDUCAM_FIFO_SIZE3				0x44

#define ARDUCAM_BURST_FIFO_READ			0x3C
#define ARDUCAM_SINGLE_FIFO_READ		0x3D

#include "rtos/drivers/spi/api/rtos_spi_master.h"
#include "rtos/drivers/i2c/api/rtos_i2c_master.h"

int32_t ov2640_init( rtos_spi_master_device_t* spi_dev, rtos_i2c_master_t* i2c_dev );
int32_t ov2640_configure();

void ov2640_flush_fifo();
void ov2640_clear_fifo_flag();
void ov2640_start_capture();
uint8_t ov2640_capture_done();
uint32_t ov2640_read_fifo_length();

uint8_t ov2640_spi_read( uint8_t addr );
void ov2640_spi_read_buf( uint8_t* rx_buf, int32_t rx_size, uint8_t* tx_buf, int32_t tx_size );

typedef struct program_mem
{
    uint8_t reg;
    uint8_t val;
} program_mem_t;

extern const struct program_mem OV2640_JPEG[];
extern const struct program_mem OV2640_NONCOMPRESSED[];
extern const struct program_mem OV2640_YUV422[];
extern const struct program_mem OV2640_JPEG_INIT[];
extern const struct program_mem OV2640_96x96_JPEG[];
extern const struct program_mem OV2640_160x120_JPEG[];

#endif /* OV2640_H_ */
