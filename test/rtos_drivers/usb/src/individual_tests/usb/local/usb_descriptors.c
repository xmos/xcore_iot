/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "individual_tests/usb/local/usb_descriptors.h"
#include "tusb.h"

#define XMOS_VID    0x20B1
#define TEST_PID    0x4A4D

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,

    .bDeviceClass       = TUSB_CLASS_UNSPECIFIED,
    .bDeviceSubClass    = TUSB_CLASS_UNSPECIFIED,
    .bDeviceProtocol    = TUSB_CLASS_UNSPECIFIED,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = XMOS_VID,
    .idProduct          = TEST_PID,
    .bcdDevice          = 0x0001,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const* tud_descriptor_device_cb(void)
{
    return (uint8_t const*) &desc_device;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

#define TUD_XMOS_DEVICE_CONTROL_DESC_LEN 9

#define TUD_XMOS_DEVICE_CONTROL_DESCRIPTOR(_itfnum, _stridx) \
  /* Interface */\
  9, TUSB_DESC_INTERFACE, _itfnum, 0, 0, TUSB_CLASS_VENDOR_SPECIFIC, 0x00, 0x00, _stridx

const size_t uac2_interface_descriptors_length =
        TUD_AUDIO_DESC_CLK_SRC_LEN +
        + TUD_AUDIO_DESC_INPUT_TERM_LEN
        + TUD_AUDIO_DESC_OUTPUT_TERM_LEN
        + TUD_AUDIO_DESC_INPUT_TERM_LEN
        + TUD_AUDIO_DESC_OUTPUT_TERM_LEN
        ;

const size_t uac2_total_descriptors_length =
        TUD_AUDIO_DESC_IAD_LEN +
        TUD_AUDIO_DESC_STD_AC_LEN +
        TUD_AUDIO_DESC_CS_AC_LEN +
        uac2_interface_descriptors_length +
        + TUD_AUDIO_DESC_STD_AS_INT_LEN
        + TUD_AUDIO_DESC_STD_AS_INT_LEN
        + TUD_AUDIO_DESC_CS_AS_INT_LEN
        + TUD_AUDIO_DESC_TYPE_I_FORMAT_LEN
        + TUD_AUDIO_DESC_STD_AS_ISO_EP_LEN
        + TUD_AUDIO_DESC_CS_AS_ISO_EP_LEN
        + TUD_AUDIO_DESC_STD_AS_INT_LEN
        + TUD_AUDIO_DESC_STD_AS_INT_LEN
        + TUD_AUDIO_DESC_CS_AS_INT_LEN
        + TUD_AUDIO_DESC_TYPE_I_FORMAT_LEN
        + TUD_AUDIO_DESC_STD_AS_ISO_EP_LEN
        + TUD_AUDIO_DESC_CS_AS_ISO_EP_LEN
        ;

// List of audio descriptor lengths which is required by audio driver - you need as many entries as CFG_TUD_AUDIO - unfortunately this is not possible to determine otherwise
const uint16_t tud_audio_desc_lengths[CFG_TUD_AUDIO] = {
        uac2_total_descriptors_length
};

#define CONFIG_TOTAL_LEN        (TUD_CONFIG_DESC_LEN + CFG_TUD_AUDIO * uac2_total_descriptors_length)
#define EPNUM_AUDIO   0x01

// TAKE CARE - THE NUMBER OF AUDIO STREAMING INTERFACES PER AUDIO FUNCTION MUST NOT EXCEED CFG_TUD_AUDIO_N_AS_INT - IF IT DOES INCREASE CFG_TUD_AUDIO_N_AS_INT IN tusb_config.h!

#define AUDIO_INTERFACE_STRING_INDEX 4

uint8_t const desc_configuration[] = {
    // Interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 400),

    /* Standard Interface Association Descriptor (IAD) */
    TUD_AUDIO_DESC_IAD(/*_firstitfs*/ ITF_NUM_AUDIO_CONTROL, /*_nitfs*/ 3, /*_stridx*/ 0x00),
    /* Standard AC Interface Descriptor(4.7.1) */
    TUD_AUDIO_DESC_STD_AC(/*_itfnum*/ ITF_NUM_AUDIO_CONTROL, /*_nEPs*/ 0x00, /*_stridx*/ AUDIO_INTERFACE_STRING_INDEX),
    /* Class-Specific AC Interface Header Descriptor(4.7.2) */
    TUD_AUDIO_DESC_CS_AC(/*_bcdADC*/ 0x0200, /*_category*/ AUDIO_FUNC_OTHER, /*_totallen*/ uac2_interface_descriptors_length, /*_ctrl*/ AUDIO_CS_AS_INTERFACE_CTRL_LATENCY_POS),
    /* Clock Source Descriptor(4.7.2.1) */
    TUD_AUDIO_DESC_CLK_SRC(/*_clkid*/ UAC2_ENTITY_CLOCK, /*_attr*/ AUDIO_CLOCK_SOURCE_ATT_INT_PRO_CLK, /*_ctrl*/ (AUDIO_CTRL_R << AUDIO_CLOCK_SOURCE_CTRL_CLK_VAL_POS) | (AUDIO_CTRL_R << AUDIO_CLOCK_SOURCE_CTRL_CLK_FRQ_POS), /*_assocTerm*/ 0x00,  /*_stridx*/ 0x00),

    /* Input Terminal Descriptor(4.7.2.4) */
    TUD_AUDIO_DESC_INPUT_TERM(/*_termid*/ UAC2_ENTITY_SPK_INPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_USB_STREAMING, /*_assocTerm*/ 0x00, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_nchannelslogical*/ CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_idxchannelnames*/ 0x00, /*_ctrl*/ AUDIO_CTRL_NONE, /*_stridx*/ 0x00),
    /* Output Terminal Descriptor(4.7.2.5) */
    TUD_AUDIO_DESC_OUTPUT_TERM(/*_termid*/ UAC2_ENTITY_SPK_OUTPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_OUT_GENERIC_SPEAKER, /*_assocTerm*/ 0x00, /*_srcid*/ UAC2_ENTITY_SPK_INPUT_TERMINAL, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_ctrl*/ AUDIO_CTRL_NONE, /*_stridx*/ 0x00),

    /* Input Terminal Descriptor(4.7.2.4) */
    TUD_AUDIO_DESC_INPUT_TERM(/*_termid*/ UAC2_ENTITY_MIC_INPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_IN_GENERIC_MIC, /*_assocTerm*/ 0x00, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_nchannelslogical*/ CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_idxchannelnames*/ 0x00, /*_ctrl*/ AUDIO_CTRL_NONE, /*_stridx*/ 0x00),

    /* Output Terminal Descriptor(4.7.2.5) */
    TUD_AUDIO_DESC_OUTPUT_TERM(/*_termid*/ UAC2_ENTITY_MIC_OUTPUT_TERMINAL, /*_termtype*/ AUDIO_TERM_TYPE_USB_STREAMING, /*_assocTerm*/ 0x00, /*_srcid*/ UAC2_ENTITY_MIC_INPUT_TERMINAL, /*_clkid*/ UAC2_ENTITY_CLOCK, /*_ctrl*/ AUDIO_CTRL_NONE, /*_stridx*/ 0x00),

    /* Standard AS Interface Descriptor(4.9.1) */
    /* Interface 1, Alternate 0 - default alternate setting with 0 bandwidth */
    TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ ITF_NUM_AUDIO_STREAMING_SPK, /*_altset*/ 0x00, /*_nEPs*/ 0x00, /*_stridx*/ 0x00),
    /* Standard AS Interface Descriptor(4.9.1) */
    /* Interface 1, Alternate 1 - alternate interface for data streaming */
    TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ ITF_NUM_AUDIO_STREAMING_SPK, /*_altset*/ 0x01, /*_nEPs*/ 0x01, /*_stridx*/ 0x00),
    /* Class-Specific AS Interface Descriptor(4.9.2) */
    TUD_AUDIO_DESC_CS_AS_INT(/*_termid*/ UAC2_ENTITY_SPK_INPUT_TERMINAL, /*_ctrl*/ AUDIO_CTRL_NONE, /*_formattype*/ AUDIO_FORMAT_TYPE_I, /*_formats*/ AUDIO_DATA_FORMAT_TYPE_I_PCM, /*_nchannelsphysical*/ CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_stridx*/ 0x00),
    /* Type I Format Type Descriptor(2.3.1.6 - Audio Formats) */
    TUD_AUDIO_DESC_TYPE_I_FORMAT(CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_RX, CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_RX*8),
    /* Standard AS Isochronous Audio Data Endpoint Descriptor(4.10.1.1) */
    TUD_AUDIO_DESC_STD_AS_ISO_EP(/*_ep*/ EPNUM_AUDIO, /*_attr*/ (TUSB_XFER_ISOCHRONOUS | TUSB_ISO_EP_ATT_ADAPTIVE | /*TUSB_ISO_EP_ATT_IMPLICIT_FB |*/ TUSB_ISO_EP_ATT_DATA), /*_maxEPsize*/ CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ, /*_interval*/ (CFG_TUSB_RHPORT0_MODE & OPT_MODE_HIGH_SPEED) ? 0x04 : 0x01),
    /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor(4.10.1.2) */
    TUD_AUDIO_DESC_CS_AS_ISO_EP(/*_attr*/ AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK, /*_ctrl*/ AUDIO_CTRL_NONE, /*_lockdelayunit*/ AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_UNDEFINED, /*_lockdelay*/ 0x0000),

    /* Standard AS Interface Descriptor(4.9.1) */
    /* Interface 1, Alternate 0 - default alternate setting with 0 bandwidth */
    TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ ITF_NUM_AUDIO_STREAMING_MIC, /*_altset*/ 0x00, /*_nEPs*/ 0x00, /*_stridx*/ 0x00),
    /* Standard AS Interface Descriptor(4.9.1) */
    /* Interface 1, Alternate 1 - alternate interface for data streaming */
    TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ ITF_NUM_AUDIO_STREAMING_MIC, /*_altset*/ 0x01, /*_nEPs*/ 0x01, /*_stridx*/ 0x00),
    /* Class-Specific AS Interface Descriptor(4.9.2) */
    TUD_AUDIO_DESC_CS_AS_INT(/*_termid*/ UAC2_ENTITY_MIC_OUTPUT_TERMINAL, /*_ctrl*/ AUDIO_CTRL_NONE, /*_formattype*/ AUDIO_FORMAT_TYPE_I, /*_formats*/ AUDIO_DATA_FORMAT_TYPE_I_PCM, /*_nchannelsphysical*/ CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_stridx*/ 0x00),
    /* Type I Format Type Descriptor(2.3.1.6 - Audio Formats) */
    TUD_AUDIO_DESC_TYPE_I_FORMAT(CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX, CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX*8),
    /* Standard AS Isochronous Audio Data Endpoint Descriptor(4.10.1.1) */
    TUD_AUDIO_DESC_STD_AS_ISO_EP(/*_ep*/ 0x80 | EPNUM_AUDIO, /*_attr*/ (TUSB_XFER_ISOCHRONOUS | TUSB_ISO_EP_ATT_ADAPTIVE | /*TUSB_ISO_EP_ATT_IMPLICIT_FB |*/ TUSB_ISO_EP_ATT_DATA), /*_maxEPsize*/ CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ, /*_interval*/ (CFG_TUSB_RHPORT0_MODE & OPT_MODE_HIGH_SPEED) ? 0x04 : 0x01),
    /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor(4.10.1.2) */
    TUD_AUDIO_DESC_CS_AS_ISO_EP(/*_attr*/ AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK, /*_ctrl*/ AUDIO_CTRL_NONE, /*_lockdelayunit*/ AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_UNDEFINED, /*_lockdelay*/ 0x0000),
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const* tud_descriptor_configuration_cb(uint8_t index)
{
    (void) index; // for multiple configurations
    return desc_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// array of pointer to string descriptors
char const *string_desc_arr[] = {(const char[]) {0x09, 0x04}, // 0: is supported language is English (0x0409)
        "XMOS",                    // 1: Manufacturer
        "DRIVERTEST",              // 2: Product
        "123456",                  // 3: Serials
        "UAC2",                    // 4: Audio Interface
        };

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index,
                                         uint16_t langid)
{
    (void) langid;

    uint8_t chr_count;

    if (index == 0) {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    } else {
        // Convert ASCII string into UTF-16

        if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])))
            return NULL;

        const char *str = string_desc_arr[index];

        // Cap at max char
        chr_count = strlen(str);
        if (chr_count > 31)
            chr_count = 31;

        for (uint8_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = str[i];
        }
    }

    // first byte is length (including header), second byte is string type
    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

    return _desc_str;
}
