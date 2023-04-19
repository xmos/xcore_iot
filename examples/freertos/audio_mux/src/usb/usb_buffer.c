// Copyright 2023 XMOS LIMITED.

#include <xcore/chanend.h>
#include <xcore/channel.h>
#include <xcore/channel_transaction.h>
#include <xcore/hwtimer.h>
#include <xcore/interrupt_wrappers.h>
#include <xcore/interrupt.h>
#include <xcore/triggerable.h>
#include <xcore/lock.h>
#include <xcore/hwtimer.h>
#include <xcore/select.h>
#include <xcore/parallel.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define DEBUG_UNIT USB_BUFFER
#ifndef DEBUG_PRINT_ENABLE_USB_BUFFER
    #define DEBUG_PRINT_ENABLE_USB_BUFFER 0
#endif
#include "debug_print.h"

/* App headers */
#include "xud_device.h"
#include "usb_buffer.h"
#include "fifo_impl.h"
#include "sample_packing.h"
// #include "sw_pll.h"
// #include "tile_common.h"
#include "tusb_config.h"

/* Config headers for sw_pll */
// #include "fractions_1000ppm.h"
// #include "register_setup_1000ppm.h"


// static inline void exchange_samples_with_audio( chanend_t chan_usb_to_audio,
//                                                 int32_t samples_host_to_device[SAMPLE_COLLECTION_SIZE][appconfUSB_CHANNELS_OUT],
//                                                 int32_t samples_device_to_host[SAMPLE_COLLECTION_SIZE][appconfUSB_CHANNELS_IN])
// {
//     const unsigned upsample_to_usb_ratio = 3;

//     transacting_chanend_t ct_frame = chan_init_transaction_slave(chan_usb_to_audio);
//     for(int smp = 0; smp < upsample_to_usb_ratio; smp++){
//         for(int ch = 0; ch < appconfUSB_CHANNELS_OUT; ch++){
//             int32_t sample = t_chan_in_word(&ct_frame);
//             samples_device_to_host[smp][ch] = sample;
//         }
//     }
//     for(int smp = 0; smp < upsample_to_usb_ratio; smp++){
//         for(int ch = 0; ch < appconfUSB_CHANNELS_IN; ch++){
//             int32_t sample = samples_host_to_device[smp][ch];
//             t_chan_out_word(&ct_frame, sample);
//         }
//     }
//     chan_complete_transaction(ct_frame);
// }

// static void init_adaptive_pll(sw_pll_state_t *sw_pll)
// {
//     port_t p_mic_clk_count = PORT_MIC_CLK_COUNT;  // Used internally by sw_pll

//     // Create clock from mclk port and use it to clock the p_mclk_count port which will count MCLKs.
//     port_enable(p_mic_clk_count);

//     // Allow p_mclk_count to count mic clocks (3.072MHz derivied from MCLK)
//     xclock_t clk_mic_clk = MIC_ARRAY_CONFIG_CLOCK_BLOCK_A;
//     port_set_clock(p_mic_clk_count, clk_mic_clk);

//     sw_pll_init(sw_pll,
//                 SW_PLL_15Q16(0.3), //TODO tune me
//                 SW_PLL_15Q16(0.00003),
//                 PLL_CONTROL_LOOP_COUNT_UA,
//                 PLL_RATIO,
//                 0,
//                 frac_values_90,
//                 SW_PLL_NUM_LUT_ENTRIES(frac_values_90),
//                 APP_PLL_CTL_REG,
//                 APP_PLL_DIV_REG,
//                 SW_PLL_NUM_LUT_ENTRIES(frac_values_90) / 2,
//                 PLL_PPM_RANGE);

//     debug_printf("Using SW PLL to track Adaptive USB input\n");
// }

// Convert and also check valid. If invlaid, it will return default to 16b / 2
static unsigned bitdepth_to_subslot(unsigned bitdepth)
{
    if(bitdepth % 8 != 0 || bitdepth < 16 || bitdepth > 32)
    {
        return 2;
    } else {
        return bitdepth / 8;
    }
}

// Audio buffer controls
#define DIV_ROUND_UP(n, d) (n / d + 1)  //Always rounds up to the next integer. Needed for 48001Hz case etc.
#define BIGGEST(a, b) (a > b ? a : b)

#define AUDIO_CLASS                       1
#define SOF_FREQ_HZ                       (8000 - ((2 - AUDIO_CLASS) * 7000) ) //1000 for FS or 8000 for HS
#define SOF_PERIOD                        ((unsigned)((XS1_TIMER_MHZ * 1e6) / SOF_FREQ_HZ))

#define appconfUSB_IN_OUT_NOMINAL_HZ appconfUSB_AUDIO_SAMPLE_RATE
#define appconfUSB_CHANNELS_OUT      CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX
#define appconfUSB_CHANNELS_IN       CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX

//Defines for endpoint buffer sizes. Samples is total number of samples across all channels
#define MAX_OUT_SAMPLES_PER_SOF_PERIOD    (DIV_ROUND_UP(appconfUSB_IN_OUT_NOMINAL_HZ, SOF_FREQ_HZ) * appconfUSB_CHANNELS_OUT)
#define NOM_OUT_SAMPLES_PER_SOF_PERIOD    ((appconfUSB_IN_OUT_NOMINAL_HZ / SOF_FREQ_HZ) * appconfUSB_CHANNELS_OUT)
#define MAX_IN_SAMPLES_PER_SOF_PERIOD     (DIV_ROUND_UP(appconfUSB_IN_OUT_NOMINAL_HZ, SOF_FREQ_HZ) * appconfUSB_CHANNELS_IN)
#define NOM_IN_SAMPLES_PER_SOF_PERIOD     ((appconfUSB_IN_OUT_NOMINAL_HZ / SOF_FREQ_HZ) * appconfUSB_CHANNELS_IN)
#define MAX_OUTPUT_SLOT_SIZE              4
#define MAX_INPUT_SLOT_SIZE               4

#define OUT_AUDIO_BUFFER_SIZE_BYTES       (MAX_OUT_SAMPLES_PER_SOF_PERIOD * MAX_OUTPUT_SLOT_SIZE)
#define IN_AUDIO_BUFFER_SIZE_BYTES        (MAX_IN_SAMPLES_PER_SOF_PERIOD * MAX_INPUT_SLOT_SIZE)

#define OUT_FIFO_LENGTH                   (4 * MAX_OUT_SAMPLES_PER_SOF_PERIOD)
#define IN_FIFO_LENGTH                    (4 * MAX_IN_SAMPLES_PER_SOF_PERIOD)

#define OUT_FIFO_TARGET                   (appconfUSB_CHANNELS_OUT * ((OUT_FIFO_LENGTH / 2) / appconfUSB_CHANNELS_OUT))
#define IN_FIFO_TARGET                    (appconfUSB_CHANNELS_IN * ((IN_FIFO_LENGTH / 2) / appconfUSB_CHANNELS_IN))

#define EP0_PROXY_BUF_LEN (1024)



DEFINE_INTERRUPT_PERMITTED (test_isr_grp_none, void, usb_buffer, usb_buffer_args_t *args)//, rtos_ep0_proxy_t *ep0_proxy_ctx)
/* USB buffer and EP0 proxy Task */
{
    chanend_t chan_ep_audio_out = args->chan_ep_audio_out;
    chanend_t chan_ep_audio_in = args->chan_ep_audio_in;
    // chanend_t chan_usb_to_audio = args->chan_usb_to_audio;
    // chanend_t chan_sof = args->chan_sof;

    // It seems that triggerable_setup_interrupt_callback needs to be called from the same thread on which the isr will be
    // scheduled. I can't get it to work unless triggerable_setup_interrupt_callback is called from this thread.
    // triggerable_setup_interrupt_callback(ep0_proxy_ctx->chan_ep0_out, ep0_proxy_ctx, INTERRUPT_CALLBACK(ep0_setup_pkt_recv_isr));

    // USB Audio vars
    uint8_t buffer_audio_ep_out[OUT_AUDIO_BUFFER_SIZE_BYTES];
    uint8_t buffer_audio_ep_in[IN_AUDIO_BUFFER_SIZE_BYTES];

    unsigned num_samples_received_from_host = 0;
    unsigned num_samples_to_send_to_host = 0;

    // unsigned input_interface_num = 0; // TODO add logic for streaming/not streaming
    // unsigned output_interface_num = 0;

    uint8_t bit_depth_in = 16;
    uint8_t bit_depth_out = 16;
    //get_usb_bit_depth(&bit_depth_in, &bit_depth_out);
    const unsigned in_subslot_size = bitdepth_to_subslot(bit_depth_in);
    const unsigned out_subslot_size = bitdepth_to_subslot(bit_depth_out);

    // FIFOs from EP buffers to audio
    // These are the storage arrays for the FIFO
    int8_t host_to_device_fifo_storage[MAX_OUTPUT_SLOT_SIZE * OUT_FIFO_LENGTH]; //OUT_FIFO_LENGTH is in samples. Allocate memory enough to store highest bit-res(32 bit) samples
    int8_t device_to_host_fifo_storage[MAX_INPUT_SLOT_SIZE * IN_FIFO_LENGTH];  //IN_FIFO_LENGTH is in samples. Allocate memory enough to store highest bit-res(32 bit) samples

    mem_fifo_t host_to_device_fifo = {
      OUT_FIFO_LENGTH, //FIFO size in samples
      host_to_device_fifo_storage,
      0,
      0,
    };

    mem_fifo_t device_to_host_fifo = {
      IN_FIFO_LENGTH, //FIFO size in samples
      device_to_host_fifo_storage,
      0,
      0,
    };

    // Audio samples for exchanging with audio manager
    // int32_t samples_device_to_host[SAMPLE_COLLECTION_SIZE][appconfUSB_CHANNELS_IN] = {{0}};
    // int32_t samples_host_to_device[SAMPLE_COLLECTION_SIZE][appconfUSB_CHANNELS_OUT] = {{0}};


    // Control loop vars
    // sw_pll_state_t sw_pll = {0};
    // init_adaptive_pll(&sw_pll);
    // int host_to_device_fill_level = 0;


    // Audio endpoints
    XUD_ep ep_audio_out = XUD_InitEp(chan_ep_audio_out);
    XUD_ep ep_audio_in = XUD_InitEp(chan_ep_audio_in);

    XUD_SetReady_OutPtr(ep_audio_out, (unsigned)buffer_audio_ep_out);
    XUD_SetReady_InPtr(ep_audio_in, (unsigned)buffer_audio_ep_in, MAX_IN_SAMPLES_PER_SOF_PERIOD * MAX_INPUT_SLOT_SIZE * appconfUSB_CHANNELS_IN);

    const unsigned max_number_of_samples_to_exchange_with_audio = 3;
    unsigned d2h_num_samples_to_exchange_with_audio = 3;
    unsigned h2d_num_samples_to_exchange_with_audio = 3;

    // General state
    bool usb_enumerated = false; // This holds off exchange with audio until we have finished enumeration

    // TODO this could eventually be used for handling an audio exchange ISR
    bool trigger_default_case = false;

    // TODO we can remove these when input_interface_num and output_interface_num are implemented
    bool d2h_full = false;
    bool d2h_empty = false;
    bool h2d_full = false;
    bool h2d_empty = false;

    // Main event loop
    SELECT_RES(
        CASE_THEN(chan_ep_audio_out, event_usb_audio_out),
        CASE_THEN(chan_ep_audio_in, event_usb_audio_in),
        // CASE_THEN(chan_usb_to_audio, event_audio_exchange),
        // CASE_THEN(chan_sof, event_sof),
        DEFAULT_GUARD_THEN(trigger_default_case, default_handler)
    )
    {
        // Host to device
        event_usb_audio_out:
        {
            
            unsigned num_bytes_received_from_host = 0;
            XUD_GetBuffer(ep_audio_out, buffer_audio_ep_out, &num_bytes_received_from_host);

            unsigned total_num_samples_received_from_host = num_bytes_received_from_host / out_subslot_size;
            num_samples_received_from_host = total_num_samples_received_from_host / appconfUSB_CHANNELS_OUT; 
            // printintln(num_samples_received_from_host);
            // printintln(*(((int16_t*)buffer_audio_ep_out)));

            fifo_ret_t ret = fifo_block_push_fast(&host_to_device_fifo, buffer_audio_ep_out, total_num_samples_received_from_host, out_subslot_size);
            if (ret != FIFO_SUCCESS) {
                if(!h2d_full){
                    debug_printf("h2d full\n");
                    h2d_full = true;
                }
            } else {
                h2d_full = false;
            }
                    
            // TODO use output_interface_num
            // if(!usb_enumerated)
            // {
            //     chan_out_word(chan_usb_to_audio, 0);
            //     usb_enumerated = true;
            // }

            // TODO make better - VERY basic adaptive USB
            // Get fill level at this point so we don't get loads of jitter
            // host_to_device_fill_level = fifo_get_fill_relative_half(&host_to_device_fifo) / appconfUSB_CHANNELS_OUT;
            // // printintln(host_to_device_fill_level);
            // sw_pll_lock_status_t lock_status = sw_pll_do_control_from_error(&sw_pll, -host_to_device_fill_level);
            // (void) lock_status; // Possibly use this for control later

            // // Adaptive USB - we match output rate with input rate
            num_samples_to_send_to_host = num_samples_received_from_host;

            //Mark EP as ready for next frame from host
            XUD_SetReady_OutPtr(ep_audio_out, (unsigned)buffer_audio_ep_out);
        }
        continue;

        // Device to host
        event_usb_audio_in:
        {
            unsigned total_num_samples_to_send_to_host = num_samples_to_send_to_host * appconfUSB_CHANNELS_IN;
            fifo_ret_t ret = fifo_block_pop_fast(&host_to_device_fifo, buffer_audio_ep_in, total_num_samples_to_send_to_host, in_subslot_size);
            if (ret != FIFO_SUCCESS) {
                memset(buffer_audio_ep_in, 0, sizeof(buffer_audio_ep_in));
                if(!d2h_empty){
                    debug_printf("d2h empty\n");
                    d2h_empty = true;
                }
            } else {
                d2h_empty = false;
            }

            unsigned num_bytes_to_send_to_host = total_num_samples_to_send_to_host * in_subslot_size;
            XUD_SetBuffer(ep_audio_in, buffer_audio_ep_in, num_bytes_to_send_to_host);

            XUD_SetReady_InPtr(ep_audio_in, (unsigned) buffer_audio_ep_in, num_bytes_to_send_to_host);
        }
        continue;

        // event_audio_exchange:
        // {
        //     // Do exchange first so we block audio for the least possible time
        //     exchange_samples_with_audio(chan_usb_to_audio, samples_host_to_device, samples_device_to_host);

        //     // Samples to host to device
        //     uint8_t samples_in_packed[MAX_INPUT_SLOT_SIZE * appconfUSB_CHANNELS_IN * max_number_of_samples_to_exchange_with_audio];
        //     pack_samples_to_buff(&samples_device_to_host[0][0], d2h_num_samples_to_exchange_with_audio * appconfUSB_CHANNELS_IN, in_subslot_size, &samples_in_packed[0]);
        //     fifo_ret_t ret = fifo_block_push_fast(&device_to_host_fifo, &samples_in_packed[0], d2h_num_samples_to_exchange_with_audio * appconfUSB_CHANNELS_IN, in_subslot_size);


        //     if (ret != FIFO_SUCCESS) 
        //     {
        //         if(!d2h_full) 
        //         {
        //             debug_printf("d2h full\n");
        //             d2h_full = true;
        //         }
        //     } else {
        //         d2h_full = false;
        //     }


        //     // Samples from host to device
        //     uint8_t samples_out_packed[MAX_OUTPUT_SLOT_SIZE * appconfUSB_CHANNELS_OUT * max_number_of_samples_to_exchange_with_audio];
        //     ret = fifo_block_pop_fast(&host_to_device_fifo, &samples_out_packed[0], h2d_num_samples_to_exchange_with_audio * appconfUSB_CHANNELS_OUT, out_subslot_size);
        //     if (ret != FIFO_SUCCESS) 
        //     {
        //         if(!h2d_empty) 
        //         {
        //             debug_printf("h2d empty\n");
        //             h2d_empty = true;
        //         }
        //         memset(samples_host_to_device, 0, sizeof(samples_host_to_device));
        //     } else {
        //         unpack_buff_to_samples(&samples_out_packed[0], h2d_num_samples_to_exchange_with_audio * appconfUSB_CHANNELS_OUT, out_subslot_size, &samples_host_to_device[0][0]);
        //         h2d_empty = false;
        //     }


        //     // Alive tick
        //     static unsigned count = 0;
        //     count++;
        //     if(count > 16000)
        //     {
        //         debug_printf(".\n");
        //         count = 0;
        //     }
        // }
        // continue;

        // event_sof:
        // {
        //     // just service for now - may add adaptive maths here later
        //     chanend_in_word(chan_sof);
        // }
        // continue;

        default_handler:
            trigger_default_case = false;
        continue;
    }
}