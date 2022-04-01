// CopyriGTH 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
/* System headers */
#include <platform.h>
#include <xs1.h>
#include <string.h>
#include <xcore/assert.h>
#include <xcore/chanend.h>
#include <xcore/channel.h>
#include <xcore/channel_streaming.h>
#include <xcore/parallel.h>
#include <xcore/port.h>
#include <xcore/hwtimer.h>
#include <xcore/triggerable.h>

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "wav_utils.h"
#include "equaliser.h"

#define NUM_CHANNELS 1
#define IN_WAV_FILE_NAME    "input.wav"
#define OUT_WAV_FILE_NAME   "output.wav"


// initialise threads
DECLARE_JOB(read, (chanend_t, FILE *, unsigned, unsigned));
DECLARE_JOB(equalise, (chanend_t, chanend_t, equaliser_t *, unsigned));
DECLARE_JOB(write, (chanend_t, FILE *, unsigned, unsigned));

void read(chanend_t chan, FILE * input_file, unsigned bytes_per_frame, unsigned block_count){
    int total_num_samples = FILTER_FRAME_LENGTH * NUM_CHANNELS;
    int32_t buffer[total_num_samples];
    for(int v = 0; v < block_count; v++){
        fread(buffer, bytes_per_frame * total_num_samples, 1, input_file);

        chan_out_buf_word(chan, (uint32_t*)buffer, total_num_samples);
    }
}

void equalise(chanend_t read_chan, chanend_t write_chan, equaliser_t * eq, unsigned block_count){
    int total_num_samples = FILTER_FRAME_LENGTH * NUM_CHANNELS;
    int32_t DWORD_ALIGNED buffer[total_num_samples];
    for(int v = 0; v < block_count; v++){
        chan_in_buf_word(read_chan, (uint32_t*)buffer, total_num_samples);

        for(unsigned ch = 0; ch < NUM_CHANNELS; ch++){
            eq_process_frame(eq, &buffer[ch * FILTER_FRAME_LENGTH]);
        }

        chan_out_buf_word(write_chan, (uint32_t*)buffer, total_num_samples);
    }
}

void write(chanend_t chan, FILE * output_file, unsigned bytes_per_frame, unsigned block_count){
    int total_num_samples = FILTER_FRAME_LENGTH * NUM_CHANNELS;
    int32_t buffer[total_num_samples];
    for(int v = 0; v < block_count; v++){
        chan_in_buf_word(chan, (uint32_t*)buffer, total_num_samples);

        fwrite(buffer, bytes_per_frame * total_num_samples, 1, output_file);
    }
}

int main(){

    char * input_file_name = IN_WAV_FILE_NAME;
    char * output_file_name = OUT_WAV_FILE_NAME;

    FILE * input_file = fopen(input_file_name, "rb");
    if (input_file == NULL){
        printf("Error: input file %s is not found\n", input_file_name);
        _Exit(1);
    }

    wav_header input_header_struct, output_header_struct;
    unsigned input_header_size;

    // ensure input is an appropriate wav file
    if (get_wav_header_details(input_file, &input_header_struct, &input_header_size) != 0) {
        printf("error in get_wav_header_details()\n");
        _Exit(1);
    }
    // ensure input is a 32 bit wav file
    if(input_header_struct.bit_depth != 32){
        printf("Error: unsupported wav bit depth (%d) for %s file. Only 32 supported\n", input_header_struct.bit_depth, input_file_name);
        _Exit(1);
    }
    // ensure input wav file contains correct number of channels 
    if(input_header_struct.num_channels != NUM_CHANNELS){
        printf("Error: wav num channels(%d) does not match app(%u)\n", input_header_struct.num_channels, NUM_CHANNELS);
        _Exit(1);
    }
    // ensure input file has a 16kHz sample rate
    if(input_header_struct.sample_rate != 16000){
        printf("Error: filter only supports 16kHz sample rate, input file sample rate is %d\n", input_header_struct.sample_rate);
        _Exit(1);
    }

    FILE * output_file = fopen(output_file_name, "wb");
    if (output_file == NULL){
        printf("Error: output file %s is not found\n", output_file_name);
        _Exit(1);
    }

    // create the output wav header
    unsigned frame_count = wav_get_num_frames(&input_header_struct);

    wav_form_header(&output_header_struct,
                    input_header_struct.audio_format,
                    NUM_CHANNELS,
                    input_header_struct.sample_rate,
                    input_header_struct.bit_depth,
                    frame_count);
    // write the header to the output file (sets the current pointer to the start of the data section)
    fwrite(&output_header_struct, WAV_HEADER_BYTES, 1, output_file);
    // set the input file pointer to the start o the data section
    fseek(input_file, input_header_size, SEEK_SET);

    unsigned bytes_per_frame = wav_get_num_bytes_per_frame(&input_header_struct);
    unsigned block_count = frame_count / FILTER_FRAME_LENGTH;

    // initialise equaliser
    equaliser_t eq;
    eq_init(&eq);
    /*for(int v = 11; v < 28; v++){
        eq.band_dBgain[v] = -6.0;
    }*/
    eq_get_biquads(&eq);

    // inialise chennels to communicate between threads
    channel_t read_chan = chan_alloc();
    channel_t write_chan = chan_alloc();
    // start threads
    PAR_JOBS(
        PJOB(read, (read_chan.end_a, input_file, bytes_per_frame, block_count)),
        PJOB(equalise, (read_chan.end_b, write_chan.end_a, &eq, block_count)),
        PJOB(write, (write_chan.end_b, output_file, bytes_per_frame, block_count))
    );

    fclose(input_file);
    fclose(output_file);
    return 0;
}
