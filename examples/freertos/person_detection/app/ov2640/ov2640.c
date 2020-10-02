// Copyright (c) 2020, XMOS Ltd, All rights reserved

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library headers */
#include "soc.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "i2c_driver.h"
#include "spi_master_driver.h"

/* App headers */
#include "ov2640.h"
#include "app_conf.h"

typedef struct {
    uint8_t initialized;
    uint8_t device_addr;
    soc_peripheral_t spi_dev;
    soc_peripheral_t i2c_dev;
} ov2640_host_t;

#define DELAY_MS( ms )  vTaskDelay( pdMS_TO_TICKS( ms ) )

static ov2640_host_t ov2640_host_ctx = { pdFALSE, 0, NULL, NULL };

static i2c_regop_res_t ov2640_i2c_write_reg( uint8_t reg, uint8_t val )
{
    i2c_regop_res_t retval = 0;

    if( ov2640_host_ctx.i2c_dev != NULL )
    {
        retval = i2c_driver_write_reg( ov2640_host_ctx.i2c_dev, ov2640_host_ctx.device_addr, reg, val );
    }

    return retval;
}

static uint8_t ov2640_i2c_read_reg( uint8_t reg, uint8_t *val, i2c_regop_res_t *res )
{
    uint8_t retval = 0;

    if( ov2640_host_ctx.i2c_dev != NULL )
    {
        *val = i2c_driver_read_reg( ov2640_host_ctx.i2c_dev, ov2640_host_ctx.device_addr, reg, res );
        if( *res == I2C_REGOP_SUCCESS )
        {
            retval = 1;
        }
    }

    return retval;
}

static void ov2640_spi_write( uint8_t addr, uint8_t val )
{
    uint8_t tx_buf[2];
    tx_buf[0] = addr | 0x80;
    tx_buf[1] = val;

    spi_transaction( ov2640_host_ctx.spi_dev, NULL, 0, &tx_buf[0], 2 );
}

uint8_t ov2640_spi_read( uint8_t addr )
{
    uint8_t retval = 0;

    uint8_t rx_buf[2] = { 0x00, 0x00 };
    uint8_t tx_buf[2];
    tx_buf[0] = addr & 0x7F;
    tx_buf[1] = 0x00;

    spi_transaction( ov2640_host_ctx.spi_dev, &rx_buf[0], 2, &tx_buf[0], 2 );

    retval = rx_buf[1];

    return retval;
}

void ov2640_spi_read_buf( uint8_t* rx_buf, int32_t rx_size, uint8_t* tx_buf, int32_t tx_size )
{
    spi_transaction( ov2640_host_ctx.spi_dev, rx_buf, rx_size, tx_buf, tx_size );
}

static void ov2640_spi_clear_bit( uint8_t addr, uint8_t bit )
{
	uint8_t temp;
	temp = ov2640_spi_read( addr );
	ov2640_spi_write( addr, temp & (~bit) );
}

static void ov2640_spi_set_bit( uint8_t addr, uint8_t bit )
{
	uint8_t temp;
    temp = ov2640_spi_read( addr );
    ov2640_spi_write( addr, temp | bit );
}

static uint8_t ov2640_spi_get_bit( uint8_t addr, uint8_t bit )
{
	uint8_t temp;
    temp = ov2640_spi_read( addr );
    temp &= bit;
    return temp;
}

static void ov2640_reg_jpeg_init()
{
    int i = 0;
    i2c_regop_res_t regop_res;
    do
    {
        regop_res = ov2640_i2c_write_reg( OV2640_JPEG_INIT[i].reg, OV2640_JPEG_INIT[i].val );
        if( regop_res != I2C_REGOP_SUCCESS )
        {
            debug_printf("I2C operation failed.\n");
        }
        i++;
    }
    while( ( OV2640_JPEG_INIT[i].reg != 0xFF ) | ( OV2640_JPEG_INIT[i].val != 0xFF ) );
}

static void ov2640_reg_yuv()
{
    int i = 0;
    i2c_regop_res_t regop_res;
    do
    {
        regop_res = ov2640_i2c_write_reg( OV2640_YUV422[i].reg, OV2640_YUV422[i].val );
        if( regop_res != I2C_REGOP_SUCCESS )
        {
            debug_printf("I2C operation failed.\n");
        }
        i++;
    }
    while( ( OV2640_YUV422[i].reg != 0xFF ) | ( OV2640_YUV422[i].val != 0xFF ) );
}

static void ov2640_reg_no_jpeg_compression()
{
    int i = 0;
    i2c_regop_res_t regop_res;
    do
    {
        regop_res = ov2640_i2c_write_reg( OV2640_NONCOMPRESSED[i].reg, OV2640_NONCOMPRESSED[i].val );
        if( regop_res != I2C_REGOP_SUCCESS )
        {
            debug_printf("I2C operation failed.\n");
        }
        i++;
    }
    while( ( OV2640_NONCOMPRESSED[i].reg != 0xFF ) | ( OV2640_NONCOMPRESSED[i].val != 0xFF ) );
}

static void ov2640_reg_96x96_jpeg()
{
    int i = 0;
    i2c_regop_res_t regop_res;
    do
    {
        regop_res = ov2640_i2c_write_reg( OV2640_96x96_JPEG[i].reg, OV2640_96x96_JPEG[i].val );
        if( regop_res != I2C_REGOP_SUCCESS )
        {
            debug_printf("I2C operation failed.\n");
        }
        i++;
    }
    while( ( OV2640_96x96_JPEG[i].reg != 0xFF ) | ( OV2640_96x96_JPEG[i].val != 0xFF ) );
}

static void ov2640_reg_160x120_JPEG()
{
    int i = 0;
    i2c_regop_res_t regop_res;
    do
    {
        regop_res = ov2640_i2c_write_reg( OV2640_160x120_JPEG[i].reg, OV2640_160x120_JPEG[i].val );
        if( regop_res != I2C_REGOP_SUCCESS )
        {
            debug_printf("I2C operation failed.\n");
        }
        i++;
    }
    while( ( OV2640_160x120_JPEG[i].reg != 0xFF ) | ( OV2640_160x120_JPEG[i].val != 0xFF ) );
}

void ov2640_flush_fifo()
{
    ov2640_spi_write( ARDUCAM_FIFO, ARDUCAM_FIFO_CLEAR_MASK );
    ov2640_spi_write( ARDUCAM_FIFO, ARDUCAM_FIFO_RDPTR_RST_MASK );
    ov2640_spi_write( ARDUCAM_FIFO, ARDUCAM_FIFO_WRPTR_RST_MASK );
}

void ov2640_start_capture()
{
    ov2640_spi_write( ARDUCAM_FIFO, ARDUCAM_FIFO_START_MASK );
}

void ov2640_clear_fifo_flag()
{
    ov2640_spi_write( ARDUCAM_FIFO, ARDUCAM_FIFO_CLEAR_MASK );
}

uint8_t ov2640_capture_done()
{
    return ( ov2640_spi_read( ARDUCAM_TRIG ) & ARDUCAM_TRIG_CAP_DONE_MASK );
}

uint32_t ov2640_read_fifo_length()
{
    uint32_t len1 = ov2640_spi_read( ARDUCAM_FIFO_SIZE1 );
    uint32_t len2 = ov2640_spi_read( ARDUCAM_FIFO_SIZE2 );
    uint32_t len3 = ov2640_spi_read( ARDUCAM_FIFO_SIZE3 ) & 0x7f;
    return ( ( ( len3 << 16 ) | ( len2 << 8 ) | len1 ) & 0x07fffff );
}

int32_t ov2640_init( int32_t spi_dev_id, int32_t i2c_dev_id )
{
    int32_t retval = pdFALSE;
    i2c_regop_res_t regop_res;
    uint8_t tmp;

    do
    {
        if( ov2640_host_ctx.initialized )
        {
            debug_printf("ov2640 context has already been initialized.\n");
            break;
        }

        ov2640_host_ctx.spi_dev = spi_master_driver_init(
                                   spi_dev_id,
                                   3*2, /* 6 DMA buffers */
                                   0,/* This device's interrupts should happen on core 0 */
                                   0,
                                   NULL );

        /* Camera supports 8 MHz max */
        spi_master_device_init( ov2640_host_ctx.spi_dev,
                                0, 0, /* mode 0 */
                                8,    /* 100 MHz / (8*2) / 2 = 3.125 MHz SPI clock */
                                3,    /* 3 nanosecond cs to data minimum time */
                                0,    /* no inter-byte setup delay required */
                                0 );  /* no last clock to cs delay required */


        ov2640_host_ctx.device_addr = OV2640_I2C_ADDR;
        ov2640_host_ctx.i2c_dev = i2c_driver_init( i2c_dev_id );

        //Reset the CPLD
        ov2640_spi_write(0x07, 0x80);
        DELAY_MS( 100 );
        ov2640_spi_write(0x07, 0x00);
        DELAY_MS( 100 );

        debug_printf("Arducam Rev: 0x%x\n", ov2640_spi_read( ARDUCAM_REV ) );

        /* Test that SPI is functional */
        ov2640_spi_write( ARDUCAM_TEST_REG, 0xA5 );
        tmp = ov2640_spi_read( ARDUCAM_TEST_REG );
        if( tmp != 0xA5 )
        {
            debug_printf("Test write or read to Arducam failed.\n");
            break;
        }

        /* Test that I2C is functional */
        /* Switch to bank 1 */
        regop_res = ov2640_i2c_write_reg( 0xFF, 0x01 );
        if( regop_res != I2C_REGOP_SUCCESS )
        {
            debug_printf("I2C operation failed.\n");
            break;
        }

        /* Check vid, should be 0x26 for OV2640 */
        ov2640_i2c_read_reg( OV2640_CHIPID_HIGH, &tmp, &regop_res );
        if( regop_res != I2C_REGOP_SUCCESS )
        {
            debug_printf("I2C operation failed.\n");
            break;
        }
        if( tmp != 0x26 )
        {
            debug_printf("CHIPID_HIGH is not expected value for OV2640.\n");
            break;
        }

        /* Check pid, pid should be 0x41 or 0x42 for OV2640 */
        ov2640_i2c_read_reg( OV2640_CHIPID_LOW, &tmp, &regop_res );
        if( regop_res != I2C_REGOP_SUCCESS )
        {
            debug_printf("I2C operation failed.\n");
            break;
        }
        if( !( ( tmp == 0x41 ) || ( tmp == 0x42 ) ) )
        {
            debug_printf("CHIPID_LOW is not expected value for OV2640.\n");
            break;
        }

        ov2640_host_ctx.initialized = pdTRUE;
        retval = pdTRUE;
    } while( 0 );

    return retval;
}

int32_t ov2640_configure()
{
    int32_t retval = pdFALSE;
    i2c_regop_res_t regop_res;

    do
    {
        if( !ov2640_host_ctx.initialized )
        {
            debug_printf("ov2640 context has not been initialized.\n");
            break;
        }

        /* Reset the sensor */
        regop_res = ov2640_i2c_write_reg( 0xFF, 0x01 );
        if( regop_res != I2C_REGOP_SUCCESS )
        {
            debug_printf("I2C operation failed.\n");
            break;
        }
        regop_res = ov2640_i2c_write_reg( 0x12, 0x80 );
        if( regop_res != I2C_REGOP_SUCCESS )
        {
            debug_printf("I2C operation failed.\n");
            break;
        }

        DELAY_MS( 100 );

        /* Write sensor program */
        ov2640_reg_jpeg_init();
        ov2640_reg_yuv();
        ov2640_reg_no_jpeg_compression();

        regop_res = ov2640_i2c_write_reg( 0xff, 0x01 );
        if( regop_res != I2C_REGOP_SUCCESS )
        {
            debug_printf("I2C operation failed.\n");
            break;
        }
        regop_res = ov2640_i2c_write_reg( 0x15, 0x00 );
        if( regop_res != I2C_REGOP_SUCCESS )
        {
            debug_printf("I2C operation failed.\n");
            break;
        }

        ov2640_reg_96x96_jpeg();

        /* Set Arducam mode */
        ov2640_spi_write( ARDUCAM_MODE_REG, ARDUCAM_MODE_MCU2LCD );
        retval = pdTRUE;
    } while( 0 );

    return retval;
}
