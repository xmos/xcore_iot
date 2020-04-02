// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef BITSTREAM_DEVICES_H_
#define BITSTREAM_DEVICES_H_

enum {
    BITSTREAM_INTERTILE_DEVICE_A,
    BITSTREAM_INTERTILE_DEVICE_COUNT
};
extern soc_peripheral_t bitstream_intertile_devices[BITSTREAM_INTERTILE_DEVICE_COUNT];

#endif /* BITSTREAM_DEVICES_H_ */
