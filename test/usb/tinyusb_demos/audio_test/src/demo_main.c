/*
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

/* XMOS modified demo.  Record in Audacity or comparable program
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "rtos_gpio.h"
#include "rtos_mic_array.h"
#include "platform/driver_instances.h"
#include "demo_main.h"
#include "tusb.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

#ifndef AUDIO_SAMPLE_RATE
#define AUDIO_SAMPLE_RATE   16000
#endif

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static TimerHandle_t blinky_timer_ctx = NULL;
static rtos_gpio_t *gpio_ctx = NULL;
static rtos_gpio_port_id_t button_port = 0;
static rtos_gpio_port_id_t led_port = 0;
static uint32_t led_val = 0;

// Audio controls
// Current states
bool mute[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1]; 						// +1 for master channel 0
uint16_t volume[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1]; 					// +1 for master channel 0
uint32_t sampFreq;
uint8_t clkValid;

// Range states
audio_control_range_2_n_t(1) volumeRng[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX+1]; 			// Volume range state
audio_control_range_4_n_t(1) sampleFreqRng; 						// Sample frequency range state

// Audio test data
static volatile bool interface_open = false;
static StreamBufferHandle_t sample_stream_buf;

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
    xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(BLINK_MOUNTED), 0);
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(BLINK_NOT_MOUNTED), 0);
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(BLINK_SUSPENDED), 0);
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(BLINK_MOUNTED), 0);
}

//--------------------------------------------------------------------+
// AUDIO Task
//--------------------------------------------------------------------+

void audio_task(void *arg)
{
    (void) arg;
    int32_t mic_samples[MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME][MIC_ARRAY_CONFIG_MIC_COUNT];
    int16_t *samples_to_buffer = (int16_t *) mic_samples;

    while (!tusb_inited()) {
        vTaskDelay(10);
    }

    size_t received = 0;
    do {
        received = rtos_mic_array_rx(mic_array_ctx,
                                     (int32_t**)mic_samples,
                                     MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME,
                                     0);
    } while(received != 0);

    for (;;) {
        rtos_mic_array_rx(
                mic_array_ctx,
                (int32_t**)mic_samples,
                MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME,
                portMAX_DELAY);

        if (interface_open) {
            for (int i = 0; i < MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME; i++) {
                samples_to_buffer[i] = mic_samples[i][0] >> 16;
            }

            if (xStreamBufferSend(sample_stream_buf, samples_to_buffer, MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME * CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX, 0) != MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME * CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX) {
                rtos_printf("lost mic samples\n");
            }
        }
    }
}

//--------------------------------------------------------------------+
// Application Callback API Implementations
//--------------------------------------------------------------------+

// Invoked when audio class specific set request received for an EP
bool tud_audio_set_req_ep_cb(uint8_t rhport, tusb_control_request_t const * p_request, uint8_t *pBuff)
{
  (void) rhport;
  (void) pBuff;

  // We do not support any set range requests here, only current value requests
  TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t ep = TU_U16_LOW(p_request->wIndex);

  (void) channelNum; (void) ctrlSel; (void) ep;

  return false; 	// Yet not implemented
}

// Invoked when audio class specific set request received for an interface
bool tud_audio_set_req_itf_cb(uint8_t rhport, tusb_control_request_t const * p_request, uint8_t *pBuff)
{
  (void) rhport;
  (void) pBuff;

  // We do not support any set range requests here, only current value requests
  TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t itf = TU_U16_LOW(p_request->wIndex);

  (void) channelNum; (void) ctrlSel; (void) itf;

  return false; 	// Yet not implemented
}

// Invoked when audio class specific set request received for an entity
bool tud_audio_set_req_entity_cb(uint8_t rhport, tusb_control_request_t const * p_request, uint8_t *pBuff)
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
  if ( entityID == 2 )
  {
    switch ( ctrlSel )
    {
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
bool tud_audio_get_req_ep_cb(uint8_t rhport, tusb_control_request_t const * p_request)
{
  (void) rhport;

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t ep = TU_U16_LOW(p_request->wIndex);

  (void) channelNum; (void) ctrlSel; (void) ep;

  //	return tud_control_xfer(rhport, p_request, &tmp, 1);

  return false; 	// Yet not implemented
}

// Invoked when audio class specific get request received for an interface
bool tud_audio_get_req_itf_cb(uint8_t rhport, tusb_control_request_t const * p_request)
{
  (void) rhport;

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t itf = TU_U16_LOW(p_request->wIndex);

  (void) channelNum; (void) ctrlSel; (void) itf;

  return false; 	// Yet not implemented
}

// Invoked when audio class specific get request received for an entity
bool tud_audio_get_req_entity_cb(uint8_t rhport, tusb_control_request_t const * p_request)
{
  (void) rhport;

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  // uint8_t itf = TU_U16_LOW(p_request->wIndex); 			// Since we have only one audio function implemented, we do not need the itf value
  uint8_t entityID = TU_U16_HIGH(p_request->wIndex);

  // Input terminal (Microphone input)
  if (entityID == 1)
  {
    switch (ctrlSel)
    {
      case AUDIO_TE_CTRL_CONNECTOR:;
      // The terminal connector control only has a get request with only the CUR attribute.

      audio_desc_channel_cluster_t ret;

      // Those are dummy values for now
      ret.bNrChannels = 1;
      ret.bmChannelConfig = 0;
      ret.iChannelNames = 0;

      TU_LOG2("    Get terminal connector\r\n");

      return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, (void*)&ret, sizeof(ret));

      // Unknown/Unsupported control selector
      default: TU_BREAKPOINT(); return false;
    }
  }

  // Feature unit
  if (entityID == 2)
  {
    switch (ctrlSel)
    {
      case AUDIO_FU_CTRL_MUTE:
	// Audio control mute cur parameter block consists of only one byte - we thus can send it right away
	// There does not exist a range parameter block for mute
	TU_LOG2("    Get Mute of channel: %u\r\n", channelNum);
	return tud_control_xfer(rhport, p_request, &mute[channelNum], 1);

      case AUDIO_FU_CTRL_VOLUME:

	switch (p_request->bRequest)
	{
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

	    return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, (void*)&ret, sizeof(ret));

	    // Unknown/Unsupported control
	  default: TU_BREAKPOINT(); return false;
	}

	// Unknown/Unsupported control
	  default: TU_BREAKPOINT(); return false;
    }
  }

  // Clock Source unit
  if (entityID == 4)
  {
    switch (ctrlSel)
    {
      case AUDIO_CS_CTRL_SAM_FREQ:

	// channelNum is always zero in this case

	switch (p_request->bRequest)
	{
	  case AUDIO_CS_REQ_CUR:
	    TU_LOG2("    Get Sample Freq.\r\n");
	    return tud_control_xfer(rhport, p_request, &sampFreq, sizeof(sampFreq));
	  case AUDIO_CS_REQ_RANGE:
	    TU_LOG2("    Get Sample Freq. range\r\n");
	    //((tusb_control_request_t *)p_request)->wLength = 14;
	    return tud_control_xfer(rhport, p_request, &sampleFreqRng, sizeof(sampleFreqRng));

	    // Unknown/Unsupported control
	  default: TU_BREAKPOINT(); return false;
	}

	  case AUDIO_CS_CTRL_CLK_VALID:
	    // Only cur attribute exists for this request
	    TU_LOG2("    Get Sample Freq. valid\r\n");
	    return tud_control_xfer(rhport, p_request, &clkValid, sizeof(clkValid));

	    // Unknown/Unsupported control
	  default: TU_BREAKPOINT(); return false;
    }
  }

  TU_LOG2("  Unsupported entity: %d\r\n", entityID);
  return false; 	// Yet not implemented
}

bool tud_audio_tx_done_pre_load_cb(uint8_t rhport, uint8_t itf, uint8_t ep_in, uint8_t cur_alt_setting)
{
  (void) rhport;
  (void) itf;
  (void) ep_in;
  (void) cur_alt_setting;

  uint8_t buf[BYTES_PER_FRAME_EXTRA];
  size_t tx_byte_count;
  size_t bytes_available;

  interface_open = true;

  if (xStreamBufferIsFull(sample_stream_buf)) {
      xStreamBufferReset(sample_stream_buf);
      return true;
  }

  bytes_available = xStreamBufferBytesAvailable(sample_stream_buf);

  if (bytes_available > (MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME + 8) * CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX) {
      tx_byte_count = BYTES_PER_FRAME_EXTRA;
      // rtos_printf("Will send more samples to prevent an overflow (%u bytes in buffer)\n", bytes_available);
  } else {
      tx_byte_count = BYTES_PER_FRAME_NOMINAL;
  }

  /* TODO: Should ensure that a multiple of CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX is received here */
  bytes_available = xStreamBufferReceive(sample_stream_buf, buf, tx_byte_count, 0);

  tud_audio_write(buf, bytes_available);

  return true;
}

bool tud_audio_tx_done_post_load_cb(uint8_t rhport, uint16_t n_bytes_copied, uint8_t itf, uint8_t ep_in, uint8_t cur_alt_setting)
{
  (void) rhport;
  (void) n_bytes_copied;
  (void) itf;
  (void) ep_in;
  (void) cur_alt_setting;

  return true;
}

bool tud_audio_set_itf_cb(uint8_t rhport, tusb_control_request_t const * p_request)
{
    (void) rhport;
    (void) p_request;

    xassert(!interface_open);

    rtos_printf("Mic interface opened\n");

    xStreamBufferReset(sample_stream_buf);

    return true;
}

bool tud_audio_set_itf_close_EP_cb(uint8_t rhport, tusb_control_request_t const * p_request)
{
  (void) rhport;
  (void) p_request;

  interface_open = false;

  rtos_printf("Mic interface closed\n");

  return true;
}

void led_blinky_cb(TimerHandle_t xTimer)
{
    (void) xTimer;
    led_val ^= 1;

#if XCOREAI_EXPLORER
    rtos_gpio_port_out(gpio_ctx, led_port, led_val);
#else
#error No valid board was specified
#endif
}

void create_tinyusb_demo(rtos_gpio_t *ctx, unsigned priority)
{
    if (gpio_ctx == NULL) {
        gpio_ctx = ctx;

        led_port = rtos_gpio_port(PORT_LEDS);
        rtos_gpio_port_enable(gpio_ctx, led_port);
        rtos_gpio_port_out(gpio_ctx, led_port, led_val);

        // Init values
        sampFreq = AUDIO_SAMPLE_RATE;
        clkValid = 1;

        sampleFreqRng.wNumSubRanges = 1;
        sampleFreqRng.subrange[0].bMin = AUDIO_SAMPLE_RATE;
        sampleFreqRng.subrange[0].bMax = AUDIO_SAMPLE_RATE;
        sampleFreqRng.subrange[0].bRes = 0;

#if XCOREAI_EXPLORER
        button_port = rtos_gpio_port(PORT_BUTTONS);
#else
#error No valid board was specified
#endif
        rtos_gpio_port_enable(gpio_ctx, button_port);

        blinky_timer_ctx = xTimerCreate("blinky",
                                        pdMS_TO_TICKS(BLINK_NOT_MOUNTED),
                                        pdTRUE,
                                        NULL,
                                        led_blinky_cb);
        xTimerStart(blinky_timer_ctx, 0);

        sample_stream_buf = xStreamBufferCreate(1.5 * MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME * CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX,
                                                CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX);

        xTaskCreate((TaskFunction_t) audio_task,
                    "audio_task",
                    portTASK_STACK_DEPTH(audio_task),
                    NULL,
                    priority,
                    NULL);
    }
}
