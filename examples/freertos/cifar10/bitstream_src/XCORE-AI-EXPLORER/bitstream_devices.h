// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef BITSTREAM_DEVICES_H_
#define BITSTREAM_DEVICES_H_

enum {
    BITSTREAM_QSPI_FLASH_DEVICE_A,
    BITSTREAM_QSPI_FLASH_DEVICE_COUNT
};
extern soc_peripheral_t bitstream_qspi_flash_devices[BITSTREAM_QSPI_FLASH_DEVICE_COUNT];

enum {
    BITSTREAM_AI_DEVICE_A,
    BITSTREAM_AI_DEVICE_COUNT
};
extern soc_peripheral_t bitstream_ai_devices[BITSTREAM_AI_DEVICE_COUNT];

#endif /* BITSTREAM_DEVICES_H_ */
