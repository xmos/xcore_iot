// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef PLATFORM_CONF_H_
#define PLATFORM_CONF_H_

/*
 * This file contains defaults to build a basic project targetting the
 * XCORE-AI-EXPLORER board.  Users may create their own app_conf.h to override
 * any default settings.
 *
 * For a different soft tapeout design, it is recommended to create an entirely
 * different board support package.
 */

#if __has_include("app_conf.h")
#include "app_conf.h"
#endif /* __has_include("app_conf.h") */

/*****************************************/
/* Intertile Communication Configuration */
/*****************************************/
#ifndef appconfI2C_MASTER_RPC_PORT
#define appconfI2C_MASTER_RPC_PORT 10
#endif /* appconfI2C_MASTER_RPC_PORT */

#ifndef appconfI2C_MASTER_RPC_PRIORITY
#define appconfI2C_MASTER_RPC_PRIORITY (configMAX_PRIORITIES/2)
#endif /* appconfI2C_MASTER_RPC_PRIORITY */

#ifndef appconfGPIO_T0_RPC_PORT
#define appconfGPIO_T0_RPC_PORT 11
#endif /* appconfGPIO_T0_RPC_PORT */

#ifndef appconfGPIO_T1_RPC_PORT
#define appconfGPIO_T1_RPC_PORT 12
#endif /* appconfGPIO_T1_RPC_PORT */

#ifndef appconfGPIO_RPC_PRIORITY
#define appconfGPIO_RPC_PRIORITY (configMAX_PRIORITIES/2)
#endif /* appconfGPIO_RPC_PRIORITY */

/*****************************************/
/*  I/O and interrupt cores for Tile 0   */
/*****************************************/
#ifndef appconfI2C_IO_CORE
#define appconfI2C_IO_CORE                      2 /* Must be kept off core 0 with the RTOS tick ISR */
#endif /* appconfI2C_IO_CORE */

#ifndef appconfI2C_INTERRUPT_CORE
#define appconfI2C_INTERRUPT_CORE               0 /* Must be kept off I/O cores. */
#endif /* appconfI2C_INTERRUPT_CORE */

/*****************************************/
/*  I/O and interrupt cores for Tile 1   */
/*****************************************/
#ifndef appconfUART_RX_IO_CORE
#define appconfUART_RX_IO_CORE                  3 /* Must be kept off core 0 with the RTOS tick ISR */
#endif /* appconfUART_RX_IO_CORE */

#ifndef appconfUART_RX_INTERRUPT_CORE
#define appconfUART_RX_INTERRUPT_CORE           4 /* Must be kept off I/O cores. Best kept off core 0 with the tick ISR. */
#endif /* appconfUART_RX_INTERRUPT_CORE */

/*****************************************/
/*  I/O Settings                         */
/*****************************************/
#ifndef appconfUART_BAUD_RATE
#define appconfUART_BAUD_RATE 115200
#endif /* appconfUART_BAUD_RATE */

/*****************************************/
/*  I/O Task Priorities                  */
/*****************************************/
#ifndef appconfSPI_MASTER_TASK_PRIORITY
#define appconfSPI_MASTER_TASK_PRIORITY		    ( configMAX_PRIORITIES - 1 )
#endif /* appconfSPI_MASTER_TASK_PRIORITY */

#ifndef appconfQSPI_FLASH_TASK_PRIORITY
#define appconfQSPI_FLASH_TASK_PRIORITY		    ( configMAX_PRIORITIES - 1 )
#endif /* appconfQSPI_FLASH_TASK_PRIORITY */

/*****************************************/
/*  DFU Settings                         */
/*****************************************/
#define FL_QUADDEVICE_AT25FF321A \
{ \
    0,                      /* UNKNOWN */ \
    256,                    /* page size */ \
    16384,                  /* num pages */ \
    3,                      /* address size */ \
    3,                      /* log2 clock divider */ \
    0x9F,                   /* QSPI_RDID */ \
    0,                      /* id dummy bytes */ \
    3,                      /* id size in bytes */ \
    0x1F4708,               /* device id */ \
    0x20,                   /* QSPI_SE */ \
    4096,                   /* Sector erase is always 4KB */ \
    0x06,                   /* QSPI_WREN */ \
    0x04,                   /* QSPI_WRDI */ \
    PROT_TYPE_SR,           /* Protection via SR */ \
    {{0x3C,0x00},{0,0}},    /* QSPI_SP, QSPI_SU */ \
    0x02,                   /* QSPI_PP */ \
    0xEB,                   /* QSPI_READ_FAST */ \
    1,                      /* 1 read dummy byte */ \
    SECTOR_LAYOUT_REGULAR,  /* mad sectors */ \
    {4096,{0,{0}}},         /* regular sector sizes */ \
    0x05,                   /* QSPI_RDSR */ \
    0x01,                   /* QSPI_WRSR */ \
    0x01,                   /* QSPI_WIP_BIT_MASK */ \
}

#ifndef BOARD_QSPI_SPEC
/* Set up a default SPI spec if the app has not provided
 * one explicitly.
 * Note: The version checks only work in XTC Tools >15.3.0 
 *       By default FL_QUADDEVICE_AT25FF321A is used 
 */
#ifdef __XMOS_XTC_VERSION_MAJOR__
#if (__XMOS_XTC_VERSION_MAJOR__ == 15)      \
    && (__XMOS_XTC_VERSION_MINOR__ >= 2)    \
    && (__XMOS_XTC_VERSION_PATCH__ >= 0)
/* In XTC >15.2.0 some SFDP support enables a generic 
 * default spec
 */
#define BOARD_QSPI_SPEC     FL_QUADDEVICE_DEFAULT
#else
#define BOARD_QSPI_SPEC     FL_QUADDEVICE_AT25FF321A
#endif
#else
#define BOARD_QSPI_SPEC     FL_QUADDEVICE_AT25FF321A
#endif /* __XMOS_XTC_VERSION_MAJOR__ */
#endif /* BOARD_QSPI_SPEC */

#endif /* PLATFORM_CONF_H_ */
