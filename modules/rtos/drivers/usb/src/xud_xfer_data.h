// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef XUD_XFER_DATA_H_
#define XUD_XFER_DATA_H_

#include <xcore/chanend.h>
#include <xcore/assert.h>
#include <xud.h>

#define XUD_DEV_XS3 1

static inline XUD_Result_t xud_data_get_start(XUD_ep ep, uint8_t *buffer)
{
    int chan_array_ptr;
    unsigned int *ep_struct = (unsigned int *) ep;

    /* Firstly check if we have missed a USB reset - endpoint should not receive after a reset */
    if (ep_struct[9]) {
        return XUD_RES_RST;
    }

    asm ("ldw %0, %1[0]":"=r"(chan_array_ptr):"r"(ep));
    asm ("stw %0, %1[3]"::"r"(buffer),"r"(ep));
    asm ("stw %0, %1[0]"::"r"(ep),"r"(chan_array_ptr));

    return XUD_RES_OKAY;
}

static inline XUD_Result_t xud_data_get_check(chanend_t c, unsigned *length, int *is_setup)
{
    int32_t word_len;
    int8_t tail_bitlen;
    int32_t byte_len;

    /* Test if there is a RESET */
    if (chanend_test_control_token_next_byte(c)) {
        *length = 0;
        *is_setup = 0;
        return XUD_RES_RST;
    }

    /* First word on channel is packet word length */
    word_len = chanend_in_word(c);

    /* Test if this is a SETUP packet */
    if (chanend_test_control_token_next_byte(c)) {
        *is_setup = 1;
        tail_bitlen = chanend_in_control_token(c);
    } else {
        *is_setup = 0;
        tail_bitlen = chanend_in_byte(c);

        /*
         * Data packets have a 16 bit CRC. Subtract this from
         * the length.
         */
        tail_bitlen -= 16;
    }

    byte_len = (4 * word_len) + (tail_bitlen / 8);

    if (byte_len >= 0) {
        *length = byte_len;
        return XUD_RES_OKAY;
    } else {
        *length = 0;
        return XUD_RES_ERR;
    }
}

XUD_Result_t xud_data_get_finish(XUD_ep ep);
XUD_Result_t xud_setup_data_get_finish(XUD_ep ep);

static inline XUD_Result_t xud_data_set_start(XUD_ep ep, uint8_t *buffer, int len)
{
    int chan_array_ptr;
    int tmp, tmp2;
    int wordLength;
    int tailLength;
    unsigned int *ep_struct = (unsigned int *) ep;

    /* Firstly check if we have missed a USB reset - endpoint may not want to send out old data after a reset */
    if (ep_struct[9]) {
        return XUD_RES_RST;
    }

#if XUD_DEV_XS3
    /* Tail length is in bits */
    tailLength = (8 * len) & 0x1F;
#endif

    wordLength = len / sizeof(uint32_t);

#if XUD_DEV_XS3
    /* Tail length must not be 0 */
    if ((tailLength == 0) && (wordLength != 0)) {
        wordLength--;
        tailLength = 32;
    }
#else
    wordLength *= sizeof(uint32_t);
    tailLength = (32 * len) & 0x7F;
    asm ("ldw %0, %1[0]":"=r"(chan_array_ptr):"r"(ep));
#endif

#if XUD_DEV_XS3
    /* Get end off buffer address */
    asm ("add %0, %1, %2":"=r"(tmp):"r"(buffer),"r"(wordLength << 2));

    /* Produce negative offset from end of buffer */
    asm ("neg %0, %1":"=r"(tmp2):"r"(wordLength));
#else
    /* Get end off buffer address */
    asm ("add %0, %1, %2":"=r"(tmp):"r"(buffer),"r"(wordLength));

    /* Produce negative offset from end of buffer */
    asm ("neg %0, %1":"=r"(tmp2):"r"(len>>2));
#endif

    /* Store neg index */
    asm ("stw %0, %1[6]"::"r"(tmp2),"r"(ep));

    /* Store buffer pointer */
    asm ("stw %0, %1[3]"::"r"(tmp),"r"(ep));

    /*  Store tail len */
    asm ("stw %0, %1[7]"::"r"(tailLength),"r"(ep));

    /* Finally, mark ready */
#if XUD_DEV_XS3
    asm ("ldw %0, %1[0]":"=r"(chan_array_ptr):"r"(ep));
#endif
    asm ("stw %0, %1[0]"::"r"(ep),"r"(chan_array_ptr));

    return XUD_RES_OKAY;
}

XUD_Result_t xud_data_set_finish(chanend_t c, XUD_ep ep);

#endif /* XUD_XFER_DATA_H_ */
