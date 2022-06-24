// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#include "tusb_config.h"

enum {
    ITF_NUM_AUDIO_CONTROL = 0,
    ITF_NUM_AUDIO_STREAMING_SPK,
    ITF_NUM_AUDIO_STREAMING_MIC,
    ITF_NUM_TOTAL
};

// Unit numbers are arbitrary selected
#define UAC2_ENTITY_CLOCK               0x01

// Speaker path
#define UAC2_ENTITY_SPK_INPUT_TERMINAL  0x11
#define UAC2_ENTITY_SPK_FEATURE_UNIT    0x12
#define UAC2_ENTITY_SPK_OUTPUT_TERMINAL 0x13

// Microphone path
#define UAC2_ENTITY_MIC_INPUT_TERMINAL  0x21
#define UAC2_ENTITY_MIC_FEATURE_UNIT    0x22
#define UAC2_ENTITY_MIC_OUTPUT_TERMINAL 0x23

#endif /* USB_DESCRIPTORS_H_ */
