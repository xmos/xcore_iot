// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef QSPI_FLASH_DEV_CTRL_H_
#define QSPI_FLASH_DEV_CTRL_H_

typedef enum {
	qspi_flash_dev_op_read,
	qspi_flash_dev_op_write,
	qspi_flash_dev_op_erase
} qspi_flash_dev_op_t;

typedef struct {
	qspi_flash_dev_op_t operation;
	unsigned byte_address;
	unsigned byte_count;
} qspi_flash_dev_cmd_t;

#define QSPI_DEV_SWMEM_SETUP      0x01
#define QSPI_DEV_SWMEM_READ       0x02
#define QSPI_DEV_SWMEM_WRITE      0x03
#define QSPI_DEV_SWMEM_DESTROY    0x04
#define QSPI_DEV_SWMEM_FILL       0x05

#endif /* QSPI_FLASH_DEV_CTRL_H_ */
