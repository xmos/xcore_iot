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

/** Read data from a QSPI flash
 *
 *  \param dev          The QSPI flash device.
 *  \param data         The buffer to read the data into.
 *  \param address      The byte address in flash to read from.
 *  \param len          The number of bytes to read.
 */
void qspi_flash_read(
        soc_peripheral_t dev,
        uint8_t *data,
		unsigned address,
        size_t len);

/** Write data to a QSPI flash.
 *
 * \note This does not perform an erase first.
 * See qspi_flash_erase().
 *
 *  \param dev          The QSPI flash device.
 *  \param data         The buffer to write to flash.
 *  \param address      The byte address in flash to write to.
 *  \param len          The number of bytes to write.
 */
void qspi_flash_write(
        soc_peripheral_t dev,
        uint8_t *data,
		unsigned address,
        size_t len);

/**
 * Erase sectors in a QSPI flash.
 *
 * \note This erases at a 4k sector granularity.
 * All 4k sectors that the address range specified
 * resides in will be erased. For example, if \p address
 * is halfway through sector 0, and \p address +
 * \p len - 1 is halfway through sector 2, then
 * sectors 0, 1, and 2 will all be completely erased,
 * even though some of that data is outside the specified
 * address range.
 *
 *  \param dev          The QSPI flash device.
 *  \param address      The byte address in flash begin the erase.
 *                      The entire sector that contains this address
 *                      will be erased.
 *  \param len          The minimum number of bytes to erase. The actual
 *                      number of bytes erased will be greater if the
 *                      specified address range does not cover a whole
 *                      number of 4k sectors.
 */
void qspi_flash_erase(
        soc_peripheral_t dev,
		unsigned address,
        size_t len);

#endif /* QSPI_FLASH_DRIVER_H_ */
