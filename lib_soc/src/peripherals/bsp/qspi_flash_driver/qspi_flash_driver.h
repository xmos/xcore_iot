// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef QSPI_FLASH_DRIVER_H_
#define QSPI_FLASH_DRIVER_H_

#include <stdint.h>
#include "soc.h"
#include "qspi_flash_dev_ctrl.h"


/* Initialize driver*/
soc_peripheral_t qspi_flash_driver_init(
        int device_id,
        int isr_core);

/** Read from a QSPI flash
 *
 *  \param dev          The QSPI flash device
 *  \param data         The buffer to read the data into.
 *  \param address      The byte address in flash to read from.
 *  \param len          The number of bytes to read.
 */
void qspi_flash_read(
        soc_peripheral_t dev,
        uint8_t *data,
		unsigned address,
        size_t len);

/** Write to a QSPI flash
 *
 *  \param dev          The QSPI flash device
 *  \param data         The buffer to write to flash.
 *  \param address      The byte address in flash to write to.
 *  \param len          The number of bytes to write.
 */
void qspi_flash_write(
        soc_peripheral_t dev,
        uint8_t *data,
		unsigned address,
        size_t len);

void qspi_flash_erase(
        soc_peripheral_t dev,
		unsigned address,
        size_t len);

#endif /* QSPI_FLASH_DRIVER_H_ */
