// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef USB_AUDIO_H_
#define USB_AUDIO_H_

void usb_audio_send(rtos_intertile_t *intertile_ctx,
                    size_t frame_count,
                    int32_t (*processed_audio_frame)[2],
                    int32_t (*reference_audio_frame)[2],
                    int32_t (*raw_mic_audio_frame)[2]);

void usb_audio_recv(rtos_intertile_t *intertile_ctx,
                    size_t frame_count,
                    int32_t (*reference_audio_frame)[2],
                    int32_t (*raw_mic_audio_frame)[2]);

void usb_audio_init(rtos_intertile_t *intertile_ctx, unsigned priority);


#endif /* USB_AUDIO_H_ */
