// Copyright (c) 2020, XMOS Ltd, All rights reserved

/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

/* Library headers */
#include "soc.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "qspi_flash_driver.h"

#define QSPI_FLASH_FILESYSTEM_START_ADDRESS 0x100000
#define QSPI_FLASH_SECTOR_SIZE 4096

static DSTATUS drive_status[FF_VOLUMES] = {
#if FF_VOLUMES >= 10
		STA_NOINIT,
#endif
#if FF_VOLUMES >= 9
		STA_NOINIT,
#endif
#if FF_VOLUMES >= 8
		STA_NOINIT,
#endif
#if FF_VOLUMES >= 7
		STA_NOINIT,
#endif
#if FF_VOLUMES >= 6
		STA_NOINIT,
#endif
#if FF_VOLUMES >= 5
		STA_NOINIT,
#endif
#if FF_VOLUMES >= 4
		STA_NOINIT,
#endif
#if FF_VOLUMES >= 3
		STA_NOINIT,
#endif
#if FF_VOLUMES >= 2
		STA_NOINIT,
#endif
#if FF_VOLUMES >= 1
		STA_NOINIT,
#endif

};

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

__attribute__((weak))
DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = STA_NOINIT;

	if (pdrv < FF_VOLUMES) {
		stat = drive_status[pdrv];
	}

	return stat;
}


/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

__attribute__((weak))
DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;

	switch (pdrv) {
#if FF_VOLUMES >= 1
#if SOC_QSPI_FLASH_PERIPHERAL_USED
	case 0:
		qspi_flash_driver_init(
				BITSTREAM_QSPI_FLASH_DEVICE_A,
				0);
		drive_status[pdrv] &= ~STA_NOINIT;
		stat = drive_status[pdrv];
		break;
#endif
#endif
	default:
		stat = STA_NOINIT;
		break;
	}

	return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

__attribute__((weak))
DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;

	switch (pdrv) {
#if FF_VOLUMES >= 1
#if SOC_QSPI_FLASH_PERIPHERAL_USED
	case 0:
		if ((drive_status[pdrv] & ~STA_PROTECT) == 0) {
			qspi_flash_read(
					bitstream_qspi_flash_devices[BITSTREAM_QSPI_FLASH_DEVICE_A],
					buff,
					QSPI_FLASH_FILESYSTEM_START_ADDRESS + (sector * QSPI_FLASH_SECTOR_SIZE),
					count * QSPI_FLASH_SECTOR_SIZE);
			res = RES_OK;
		} else if (drive_status[pdrv] & STA_NOINIT) {
			res = RES_NOTRDY;
		} else {
			res = RES_ERROR;
		}
		break;
#endif
#endif
	default:
		res = RES_PARERR;
		break;
	}

	return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

__attribute__((weak))
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;

	switch (pdrv) {
#if FF_VOLUMES >= 1
#if SOC_QSPI_FLASH_PERIPHERAL_USED
	case 0:
		if (drive_status[pdrv] == 0) {
			qspi_flash_erase(
					bitstream_qspi_flash_devices[ BITSTREAM_QSPI_FLASH_DEVICE_A ],
					QSPI_FLASH_FILESYSTEM_START_ADDRESS + ( sector * QSPI_FLASH_SECTOR_SIZE ),
					count * QSPI_FLASH_SECTOR_SIZE );
			qspi_flash_write(
					bitstream_qspi_flash_devices[ BITSTREAM_QSPI_FLASH_DEVICE_A ],
					(uint8_t *) buff,
					QSPI_FLASH_FILESYSTEM_START_ADDRESS + ( sector * QSPI_FLASH_SECTOR_SIZE ),
					count * QSPI_FLASH_SECTOR_SIZE );
			res = RES_OK;
		} else if (drive_status[pdrv] & STA_NOINIT) {
			res = RES_NOTRDY;
		} else if (drive_status[pdrv] & STA_PROTECT) {
			res = RES_WRPRT;
		} else {
			res = RES_ERROR;
		}
		break;
#endif
#endif
	default:
		res = RES_PARERR;
		break;
	}

	return res;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

__attribute__((weak))
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;

	switch (pdrv) {
#if FF_VOLUMES >= 1
#if SOC_QSPI_FLASH_PERIPHERAL_USED
	case 0:
		if ((drive_status[pdrv] & ~STA_PROTECT) == 0) {

			switch (cmd) {
			case CTRL_SYNC:
				if (drive_status[pdrv] & STA_PROTECT) {
					res = RES_ERROR;
				} else {
					res = RES_OK;
				}
				break;

			case GET_SECTOR_COUNT:
				/* TODO: need to be able to query the flash driver for this */
				*((LBA_t *) buff) = 1024;
				res = RES_OK;
				break;

			case GET_SECTOR_SIZE:
				*((WORD *) buff) = QSPI_FLASH_SECTOR_SIZE;
				res = RES_OK;
				break;

			case GET_BLOCK_SIZE:
				*((DWORD *) buff) = QSPI_FLASH_SECTOR_SIZE;
				res = RES_OK;
				break;

			case CTRL_TRIM:
				res = RES_OK;
				break;

			default:
				res = RES_PARERR;
				break;
			}

			res = RES_OK;
		} else if (drive_status[pdrv] & STA_NOINIT) {
			res = RES_NOTRDY;
		} else {
			res = RES_ERROR;
		}
		break;
#endif
#endif
	default:
		res = RES_PARERR;
		break;
	}

	return res;
}
