/*
 * Copyright (c) 2021-2022 XMOS LIMITED. This Software is subject to the terms of the
 * XMOS Public License: Version 1
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Reinhard Panhuber
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <xcore/hwtimer.h>
#include <src.h>

#include "FreeRTOS.h"
#include "stream_buffer.h"

#include "usb_descriptors.h"
#include "tusb.h"

#include "rtos_intertile.h"

#include "audio_pipeline/audio_pipeline.h"

#include "app_conf.h"

// Audio controls
// Current states
bool mute[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1]; 						// +1 for master channel 0
uint16_t volume[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1]; 					// +1 for master channel 0
uint32_t sampFreq;
uint8_t clkValid;

// Range states
audio_control_range_2_n_t(1) volumeRng[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX+1]; 			// Volume range state
audio_control_range_4_n_t(1) sampleFreqRng; 						// Sample frequency range state

static volatile bool mic_interface_open = false;
static volatile bool spkr_interface_open = false;

static StreamBufferHandle_t samples_to_host_stream_buf;
static StreamBufferHandle_t samples_from_host_stream_buf;
static TaskHandle_t usb_audio_out_task_handle;

#define RATE_MULTIPLIER (appconfUSB_AUDIO_SAMPLE_RATE / appconfAUDIO_PIPELINE_SAMPLE_RATE)

#define USB_FRAMES_PER_VFE_FRAME (appconfAUDIO_PIPELINE_FRAME_ADVANCE / (appconfAUDIO_PIPELINE_SAMPLE_RATE / 1000))

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
    rtos_printf("USB mounted\n");
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    rtos_printf("USB unmounted\n");
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    xassert(false);
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{

}

//--------------------------------------------------------------------+
// AUDIO Task
//--------------------------------------------------------------------+

#if CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX == 2
typedef int16_t samp_t;
#elif CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX == 4
typedef int32_t samp_t;
#else
#error CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX must be either 2 or 4
#endif

void usb_audio_send(rtos_intertile_t *intertile_ctx,
                    size_t frame_count,
                    int32_t **frame_buffers,
                    size_t num_chans)
{
    samp_t usb_audio_in_frame[appconfAUDIO_PIPELINE_FRAME_ADVANCE][CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX];
    int32_t *frame_buf_ptr = (int32_t *) frame_buffers;

#if CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX == 2
    const int src_32_shift = 16;
#elif CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX == 4
    const int src_32_shift = 0;
#endif

    memset(usb_audio_in_frame, 0, sizeof(samp_t) * appconfAUDIO_PIPELINE_FRAME_ADVANCE * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX);

    xassert(frame_count == appconfAUDIO_PIPELINE_FRAME_ADVANCE);

    for(int ch=0; ch<CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX; ch++) {
        for (int i=0; i<appconfAUDIO_PIPELINE_FRAME_ADVANCE; i++) {
            if (ch < num_chans) {
                usb_audio_in_frame[i][ch] = frame_buf_ptr[i+(appconfAUDIO_PIPELINE_FRAME_ADVANCE*ch)] >> src_32_shift;
            }
        }
    }

    rtos_intertile_tx(intertile_ctx,
                      appconfUSB_AUDIO_PORT,
                      usb_audio_in_frame,
                      sizeof(usb_audio_in_frame));
}

void usb_audio_recv(rtos_intertile_t *intertile_ctx,
                    size_t frame_count,
                    int32_t **frame_buffers,
                    size_t num_chans)
{
    static samp_t usb_audio_out_frame[appconfAUDIO_PIPELINE_FRAME_ADVANCE][CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX];
    size_t bytes_received;
    int32_t *frame_buf_ptr = (int32_t *) frame_buffers;

#if CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_RX == 2
    const int src_32_shift = 16;
#elif CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_RX == 4
    const int src_32_shift = 0;
#endif

    xassert(frame_count == appconfAUDIO_PIPELINE_FRAME_ADVANCE);

#if appconfUSB_AUDIO_MODE == appconfUSB_AUDIO_RELEASE
    bytes_received = rtos_intertile_rx_len(
            intertile_ctx,
            appconfUSB_AUDIO_PORT,
            0);
#else
    bytes_received = rtos_intertile_rx_len(
            intertile_ctx,
            appconfUSB_AUDIO_PORT,
            portMAX_DELAY);
#endif

    if (bytes_received > 0) {
        xassert(bytes_received == sizeof(usb_audio_out_frame));

        rtos_intertile_rx_data(
                intertile_ctx,
                usb_audio_out_frame,
                bytes_received);
    } else {
        memset(usb_audio_out_frame, 0, sizeof(usb_audio_out_frame));
    }

    if (frame_buf_ptr != NULL) {
        memset(frame_buf_ptr, 0, sizeof(int32_t) * appconfAUDIO_PIPELINE_FRAME_ADVANCE * num_chans);
        for(int ch=0; ch<CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX; ch++) {
            for (int i=0; i<appconfAUDIO_PIPELINE_FRAME_ADVANCE; i++) {
                if (ch < num_chans) {
                    frame_buf_ptr[i+(appconfAUDIO_PIPELINE_FRAME_ADVANCE*ch)] = usb_audio_out_frame[i][0] << src_32_shift;
                }
            }
        }
    }
}

void usb_audio_in_task(void *arg)
{
    rtos_intertile_t *intertile_ctx = (rtos_intertile_t*) arg;

    while (!tusb_inited()) {
        vTaskDelay(10);
    }

    for (;;) {
        samp_t usb_audio_in_frame[appconfAUDIO_PIPELINE_FRAME_ADVANCE][CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX];
        size_t frame_length;

        frame_length = rtos_intertile_rx_len(
                intertile_ctx,
                appconfUSB_AUDIO_PORT,
                portMAX_DELAY);

        xassert(frame_length == sizeof(usb_audio_in_frame));

        rtos_intertile_rx_data(
                intertile_ctx,
                usb_audio_in_frame,
                frame_length);

        if (mic_interface_open) {
            if (xStreamBufferSend(samples_to_host_stream_buf, usb_audio_in_frame, sizeof(usb_audio_in_frame), 0) != sizeof(usb_audio_in_frame)) {
                rtos_printf("lost VFE output samples\n");
            }
        }
    }
}

void usb_audio_out_task(void *arg)
{
    rtos_intertile_t *intertile_ctx = (rtos_intertile_t*) arg;

    for (;;) {
        samp_t usb_audio_out_frame[appconfAUDIO_PIPELINE_FRAME_ADVANCE][CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX];
        size_t bytes_received = 0;

        /*
         * Only wake up when the stream buffer contains a whole audio
         * pipeline frame.
         */
        (void) ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
        bytes_received = xStreamBufferReceive(samples_from_host_stream_buf, usb_audio_out_frame, sizeof(usb_audio_out_frame), 0);

        /*
         * This shouldn't normally be zero, but it could be possible that
         * the stream buffer is reset after this task has been notified.
         */
        if (bytes_received > 0) {
            xassert(bytes_received == sizeof(usb_audio_out_frame));

            rtos_intertile_tx(
                    intertile_ctx,
                    appconfUSB_AUDIO_PORT,
                    usb_audio_out_frame,
                    bytes_received);
        }
    }
}

//--------------------------------------------------------------------+
// Application Callback API Implementations
//--------------------------------------------------------------------+

// Invoked when audio class specific set request received for an EP
bool tud_audio_set_req_ep_cb(uint8_t rhport,
                             tusb_control_request_t const *p_request,
                             uint8_t *pBuff)
{
    (void) rhport;
    (void) pBuff;

    // We do not support any set range requests here, only current value requests
    TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

    // Page 91 in UAC2 specification
    uint8_t channelNum = TU_U16_LOW(p_request->wValue);
    uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
    uint8_t ep = TU_U16_LOW(p_request->wIndex);

    (void) channelNum;
    (void) ctrlSel;
    (void) ep;

    return false; 	// Yet not implemented
}

// Invoked when audio class specific set request received for an interface
bool tud_audio_set_req_itf_cb(uint8_t rhport,
                              tusb_control_request_t const *p_request,
                              uint8_t *pBuff)
{
    (void) rhport;
    (void) pBuff;

    // We do not support any set range requests here, only current value requests
    TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

    // Page 91 in UAC2 specification
    uint8_t channelNum = TU_U16_LOW(p_request->wValue);
    uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
    uint8_t itf = TU_U16_LOW(p_request->wIndex);

    (void) channelNum;
    (void) ctrlSel;
    (void) itf;

    return false; 	// Yet not implemented
}

// Invoked when audio class specific set request received for an entity
bool tud_audio_set_req_entity_cb(uint8_t rhport,
                                 tusb_control_request_t const *p_request,
                                 uint8_t *pBuff)
{
    (void) rhport;

    // Page 91 in UAC2 specification
    uint8_t channelNum = TU_U16_LOW(p_request->wValue);
    uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
    uint8_t itf = TU_U16_LOW(p_request->wIndex);
    uint8_t entityID = TU_U16_HIGH(p_request->wIndex);

    (void) itf;

    // We do not support any set range requests here, only current value requests
    TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

    // If request is for our feature unit
    if (entityID == UAC2_ENTITY_MIC_FEATURE_UNIT) {
        switch (ctrlSel) {
        case AUDIO_FU_CTRL_MUTE:
            // Request uses format layout 1
            TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_1_t));

            mute[channelNum] = ((audio_control_cur_1_t*) pBuff)->bCur;

            TU_LOG2("    Set Mute: %d of channel: %u\r\n", mute[channelNum], channelNum);

            return true;

        case AUDIO_FU_CTRL_VOLUME:
            // Request uses format layout 2
            TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_2_t));

            volume[channelNum] = ((audio_control_cur_2_t*) pBuff)->bCur;

            TU_LOG2("    Set Volume: %d dB of channel: %u\r\n", volume[channelNum], channelNum);

            return true;

            // Unknown/Unsupported control
        default:
            TU_BREAKPOINT();
            return false;
        }
    }
    return false;    // Yet not implemented
}

// Invoked when audio class specific get request received for an EP
bool tud_audio_get_req_ep_cb(uint8_t rhport,
                             tusb_control_request_t const *p_request)
{
    (void) rhport;

    // Page 91 in UAC2 specification
    uint8_t channelNum = TU_U16_LOW(p_request->wValue);
    uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
    uint8_t ep = TU_U16_LOW(p_request->wIndex);

    (void) channelNum;
    (void) ctrlSel;
    (void) ep;

    //	return tud_control_xfer(rhport, p_request, &tmp, 1);

    return false; 	// Yet not implemented
}

// Invoked when audio class specific get request received for an interface
bool tud_audio_get_req_itf_cb(uint8_t rhport,
                              tusb_control_request_t const *p_request)
{
    (void) rhport;

    // Page 91 in UAC2 specification
    uint8_t channelNum = TU_U16_LOW(p_request->wValue);
    uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
    uint8_t itf = TU_U16_LOW(p_request->wIndex);

    (void) channelNum;
    (void) ctrlSel;
    (void) itf;

    return false; 	// Yet not implemented
}

// Invoked when audio class specific get request received for an entity
bool tud_audio_get_req_entity_cb(uint8_t rhport,
                                 tusb_control_request_t const *p_request)
{
    (void) rhport;

    // Page 91 in UAC2 specification
    uint8_t channelNum = TU_U16_LOW(p_request->wValue);
    uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
    // uint8_t itf = TU_U16_LOW(p_request->wIndex); 			// Since we have only one audio function implemented, we do not need the itf value
    uint8_t entityID = TU_U16_HIGH(p_request->wIndex);

    // Input terminal (Microphone input)
    if (entityID == UAC2_ENTITY_MIC_INPUT_TERMINAL) {
        switch (ctrlSel) {
        case AUDIO_TE_CTRL_CONNECTOR:
            ;
            // The terminal connector control only has a get request with only the CUR attribute.

            audio_desc_channel_cluster_t ret;

            // Those are dummy values for now
            ret.bNrChannels = CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX;
            ret.bmChannelConfig = 0;
            ret.iChannelNames = 0;

            TU_LOG2("    Get terminal connector\r\n");
            rtos_printf("Get terminal connector\r\n");

            return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, (void*) &ret, sizeof(ret));

            // Unknown/Unsupported control selector
        default:
            TU_BREAKPOINT();
            return false;
        }
    }

    // Feature unit
    if (entityID == UAC2_ENTITY_MIC_FEATURE_UNIT) {
        switch (ctrlSel) {
        case AUDIO_FU_CTRL_MUTE:
            // Audio control mute cur parameter block consists of only one byte - we thus can send it right away
            // There does not exist a range parameter block for mute
            TU_LOG2("    Get Mute of channel: %u\r\n", channelNum);
            return tud_control_xfer(rhport, p_request, &mute[channelNum], 1);

        case AUDIO_FU_CTRL_VOLUME:

            switch (p_request->bRequest) {
            case AUDIO_CS_REQ_CUR:
                TU_LOG2("    Get Volume of channel: %u\r\n", channelNum);
                return tud_control_xfer(rhport, p_request, &volume[channelNum], sizeof(volume[channelNum]));
            case AUDIO_CS_REQ_RANGE:
                TU_LOG2("    Get Volume range of channel: %u\r\n", channelNum);

                // Copy values - only for testing - better is version below
                audio_control_range_2_n_t(1) ret;

                ret.wNumSubRanges = 1;
                ret.subrange[0].bMin = -90; 	// -90 dB
                ret.subrange[0].bMax = 90;		// +90 dB
                ret.subrange[0].bRes = 1; 		// 1 dB steps

                return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, (void*) &ret, sizeof(ret));

                // Unknown/Unsupported control
            default:
                TU_BREAKPOINT();
                return false;
            }

            // Unknown/Unsupported control
        default:
            TU_BREAKPOINT();
            return false;
        }
    }

    // Clock Source unit
    if (entityID == UAC2_ENTITY_CLOCK) {
        switch (ctrlSel) {
        case AUDIO_CS_CTRL_SAM_FREQ:

            // channelNum is always zero in this case

            switch (p_request->bRequest) {
            case AUDIO_CS_REQ_CUR:
                TU_LOG2("    Get Sample Freq.\r\n");
                return tud_control_xfer(rhport, p_request, &sampFreq, sizeof(sampFreq));
            case AUDIO_CS_REQ_RANGE:
                TU_LOG2("    Get Sample Freq. range\r\n");
                //((tusb_control_request_t *)p_request)->wLength = 14;
                return tud_control_xfer(rhport, p_request, &sampleFreqRng, sizeof(sampleFreqRng));

                // Unknown/Unsupported control
            default:
                TU_BREAKPOINT();
                return false;
            }

        case AUDIO_CS_CTRL_CLK_VALID:
            // Only cur attribute exists for this request
            TU_LOG2("    Get Sample Freq. valid\r\n");
            return tud_control_xfer(rhport, p_request, &clkValid, sizeof(clkValid));

            // Unknown/Unsupported control
        default:
            TU_BREAKPOINT();
            return false;
        }
    }

    TU_LOG2("  Unsupported entity: %d\r\n", entityID);
    return false; 	// Yet not implemented
}

bool tud_audio_rx_done_pre_read_cb(uint8_t rhport,
                                    uint16_t n_bytes_received,
                                    uint8_t func_id,
                                    uint8_t ep_out,
                                    uint8_t cur_alt_setting)
{
    (void) rhport;
    (void) n_bytes_received;
    (void) func_id;
    (void) ep_out;
    (void) cur_alt_setting;

    return true;
}

bool tud_audio_rx_done_post_read_cb(uint8_t rhport,
                                    uint16_t n_bytes_received,
                                    uint8_t func_id,
                                    uint8_t ep_out,
                                    uint8_t cur_alt_setting)
{
  (void)rhport;

  samp_t usb_audio_frames[AUDIO_FRAMES_PER_USB_FRAME][CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX];

  const size_t stream_buffer_send_byte_count = sizeof(usb_audio_frames) / RATE_MULTIPLIER;

  /*
   * TODO: For adaptive mode (vs synchronous) then we need to
   * add support for frames that do not have the
   * nominal number of audio frames (16 or 48).
   * - What should the limit be? 2x maybe?
   * - Would we still require a whole number of audio frames?
   */
  if (n_bytes_received != sizeof(usb_audio_frames)) {
      return false;
  }

  if (!spkr_interface_open) {
      spkr_interface_open = true;
  }

  tud_audio_read(usb_audio_frames, n_bytes_received);

  if (xStreamBufferSpacesAvailable(samples_from_host_stream_buf) >= stream_buffer_send_byte_count) {

      if (RATE_MULTIPLIER == 3) {
          static int32_t src_data[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX][SRC_FF3V_FIR_NUM_PHASES][SRC_FF3V_FIR_TAPS_PER_PHASE] __attribute__((aligned (8)));
          samp_t stream_buffer_audio_frames[AUDIO_FRAMES_PER_USB_FRAME / RATE_MULTIPLIER][CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX];

          /*
           * TODO: For adaptive mode, use the actual number of audio frames
           * received, not AUDIO_FRAMES_PER_USB_FRAME. This would require
           * that a whole number of audio frames were contained within the
           * USB packet. It would also require the number of frames to be a multiple
           * of RATE_MULTIPLIER. Maybe we need to keep a static buffer around, and then
           * run this bit as is, still sending only AUDIO_FRAMES_PER_USB_FRAME at a time
           * to the stream buffer.
           */
          for (int i = 0; i < AUDIO_FRAMES_PER_USB_FRAME / RATE_MULTIPLIER; i++) {
              for (int j = 0; j < CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX; j++) {
                  int64_t sum = 0;
                  sum = src_ds3_voice_add_sample(sum, src_data[j][0], src_ff3v_fir_coefs[0], usb_audio_frames[3*i + 0][j]);
                  sum = src_ds3_voice_add_sample(sum, src_data[j][1], src_ff3v_fir_coefs[1], usb_audio_frames[3*i + 1][j]);
                  stream_buffer_audio_frames[i][j] = src_ds3_voice_add_final_sample(sum, src_data[j][2], src_ff3v_fir_coefs[2], usb_audio_frames[3*i + 2][j]);
              }
          }
          xStreamBufferSend(samples_from_host_stream_buf, stream_buffer_audio_frames, stream_buffer_send_byte_count, 0);
      } else {
          xStreamBufferSend(samples_from_host_stream_buf, usb_audio_frames, stream_buffer_send_byte_count, 0);
      }

      /*
       * Wake up the task waiting on this buffer whenever there is one more
       * USB frame worth of audio data than the amount of data required to
       * be input into the pipeline.
       *
       * This way the task will not wake up each time this task puts another
       * milliseconds of audio into the stream buffer, but rather once every
       * pipeline frame time.
       */
      const size_t buffer_notify_level = stream_buffer_send_byte_count * (1 + USB_FRAMES_PER_VFE_FRAME);

      /*
       * TODO: If the above is modified such that not exactly AUDIO_FRAMES_PER_USB_FRAME / RATE_MULTIPLIER
       * frames are written to the stream buffer at a time, then this will need to change to >=.
       */
      if (xStreamBufferBytesAvailable(samples_from_host_stream_buf) == buffer_notify_level) {
          xTaskNotifyGive(usb_audio_out_task_handle);
      }

  } else {
      rtos_printf("lost USB output samples\n");
  }

  return true;
}

bool tud_audio_tx_done_pre_load_cb(uint8_t rhport,
                                   uint8_t itf,
                                   uint8_t ep_in,
                                   uint8_t cur_alt_setting)
{
    static int ready;
    samp_t stream_buffer_audio_frames[AUDIO_FRAMES_PER_USB_FRAME / RATE_MULTIPLIER][CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX];
    size_t bytes_available;

    (void) rhport;
    (void) itf;
    (void) ep_in;
    (void) cur_alt_setting;

    if (!mic_interface_open) {
        ready = 0;
        mic_interface_open = true;
        /*
         * TODO:
         * if (RATE_MULTIPLIER == 3) { reset the data arrays };
         */
    }

    /*
     * If the buffer becomes full, reset it in an attempt to
     * maintain a good fill level again.
     */
    if (xStreamBufferIsFull(samples_to_host_stream_buf)) {
        xStreamBufferReset(samples_to_host_stream_buf);
        ready = 0;
        rtos_printf("oops buffer is full\n");
        return true;
    }

    bytes_available = xStreamBufferBytesAvailable(samples_to_host_stream_buf);
    if (bytes_available >= 2 * sizeof(samp_t) * appconfAUDIO_PIPELINE_FRAME_ADVANCE * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX) {
        /* wait until we have 2 full audio pipeline output frames in the buffer */
        ready = 1;
    }

    if (!ready) {
        return true;
    }

    if (bytes_available >= sizeof(stream_buffer_audio_frames)) {
        xStreamBufferReceive(samples_to_host_stream_buf, stream_buffer_audio_frames, sizeof(stream_buffer_audio_frames), 0);

        if (RATE_MULTIPLIER == 3) {
            static int32_t src_data[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX][SRC_FF3V_FIR_TAPS_PER_PHASE] __attribute__((aligned (8)));
            samp_t usb_audio_frames[AUDIO_FRAMES_PER_USB_FRAME][CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX];

            for (int i = 0; i < AUDIO_FRAMES_PER_USB_FRAME / RATE_MULTIPLIER; i++) {
                for (int j = 0; j < CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX; j++) {
                    usb_audio_frames[3*i + 0][j] = src_us3_voice_input_sample(src_data[j], src_ff3v_fir_coefs[2], stream_buffer_audio_frames[i][j]);
                    usb_audio_frames[3*i + 1][j] = src_us3_voice_get_next_sample(src_data[j], src_ff3v_fir_coefs[1]);
                    usb_audio_frames[3*i + 2][j] = src_us3_voice_get_next_sample(src_data[j], src_ff3v_fir_coefs[0]);
                }
            }
            tud_audio_write(usb_audio_frames, sizeof(usb_audio_frames));
        } else {
            tud_audio_write(stream_buffer_audio_frames, sizeof(stream_buffer_audio_frames));
        }
    } else {
        rtos_printf("Oops buffer is empty!\n");
    }

    return true;
}

bool tud_audio_tx_done_post_load_cb(uint8_t rhport,
                                    uint16_t n_bytes_copied,
                                    uint8_t itf,
                                    uint8_t ep_in,
                                    uint8_t cur_alt_setting)
{
    (void) rhport;
    (void) n_bytes_copied;
    (void) itf;
    (void) ep_in;
    (void) cur_alt_setting;

    return true;
}

bool tud_audio_set_itf_cb(uint8_t rhport,
                          tusb_control_request_t const *p_request)
{
    (void) rhport;
    uint8_t const itf = tu_u16_low(tu_le16toh(p_request->wIndex));
    uint8_t const alt = tu_u16_low(tu_le16toh(p_request->wValue));

#if AUDIO_OUTPUT_ENABLED
    if (itf == ITF_NUM_AUDIO_STREAMING_SPK) {
        /* In case the interface is reset without
         * closing it first */
        spkr_interface_open = false;
        xStreamBufferReset(samples_from_host_stream_buf);
    }
#endif
#if AUDIO_INPUT_ENABLED
    if (itf == ITF_NUM_AUDIO_STREAMING_MIC) {
        /* In case the interface is reset without
         * closing it first */
        mic_interface_open = false;
        xStreamBufferReset(samples_to_host_stream_buf);
    }
#endif

    rtos_printf("Set audio interface %d alt %d\n", itf, alt);

    return true;
}

bool tud_audio_set_itf_close_EP_cb(uint8_t rhport,
                                   tusb_control_request_t const *p_request)
{
    (void) rhport;
    uint8_t const itf = tu_u16_low(tu_le16toh(p_request->wIndex));
    uint8_t const alt = tu_u16_low(tu_le16toh(p_request->wValue));

#if AUDIO_OUTPUT_ENABLED
    if (itf == ITF_NUM_AUDIO_STREAMING_SPK) {
        spkr_interface_open = false;
    }
#endif
#if AUDIO_INPUT_ENABLED
    if (itf == ITF_NUM_AUDIO_STREAMING_MIC) {
        mic_interface_open = false;
    }
#endif

    rtos_printf("Close audio interface %d alt %d\n", itf, alt);

    return true;
}

void usb_audio_init(rtos_intertile_t *intertile_ctx,
                    unsigned priority)
{
    // Init values
    sampFreq = appconfUSB_AUDIO_SAMPLE_RATE;
    clkValid = 1;

    sampleFreqRng.wNumSubRanges = 1;
    sampleFreqRng.subrange[0].bMin = appconfUSB_AUDIO_SAMPLE_RATE;
    sampleFreqRng.subrange[0].bMax = appconfUSB_AUDIO_SAMPLE_RATE;
    sampleFreqRng.subrange[0].bRes = 0;

    /*
     * Note: Given the way that the USB callback notifies usb_audio_out_task,
     * the size of this buffer MUST NOT be greater than 2 VFE frames.
     */
    samples_from_host_stream_buf = xStreamBufferCreate(2 * sizeof(samp_t) * appconfAUDIO_PIPELINE_FRAME_ADVANCE * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX,
                                            0);

    /*
     * Note: The USB callback waits until there are at least 2 VFE frames
     * in this buffer before starting to send to the host, so the size of
     * this buffer MUST be AT LEAST 2 VFE frames.
     */
    samples_to_host_stream_buf = xStreamBufferCreate(2.5 * sizeof(samp_t) * appconfAUDIO_PIPELINE_FRAME_ADVANCE * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX,
                                            0);

    xTaskCreate((TaskFunction_t) usb_audio_in_task, "usb_audio_in_task", portTASK_STACK_DEPTH(usb_audio_in_task), intertile_ctx, priority, NULL);
    xTaskCreate((TaskFunction_t) usb_audio_out_task, "usb_audio_out_task", portTASK_STACK_DEPTH(usb_audio_out_task), intertile_ctx, priority, &usb_audio_out_task_handle);
}
