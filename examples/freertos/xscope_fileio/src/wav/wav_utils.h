// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef WAV_UTILS_H
#define WAV_UTILS_H

#include <stdint.h>

#include "xscope_io_device.h"

#define WAV_HEADER_BYTES 44

typedef struct wav_header {
    // RIFF Header
    char riff_header[4];    // Should be "RIFF"
    int wav_size;           // File size - 8 = data_bytes + WAV_HEADER_BYTES - 8
    char wave_header[4];    // Should be "WAVE"

    // Format Subsection
    char fmt_header[4];     // Should be "fmt "
    int fmt_chunk_size;     // Size of the rest of this subchunk
    short audio_format;
    short num_channels;
    int sample_rate;
    int byte_rate;          // sample_rate * num_channels * (bit_depth/8)
    short sample_alignment; // num_channels * (bit_depth/8)
    short bit_depth;        // bits per sample

    // Data Subsection
    char data_header[4];    // Should be "data"
    int data_bytes;         // frame count * num_channels * (bit_depth/8)
} wav_header;

int get_wav_header_details(xscope_file_t *input_file, wav_header *s, unsigned *header_size);

int wav_form_header(wav_header *header,
        short audio_format,
        short num_channels,
        int sample_rate,
        short bit_depth,
        int num_frames);

unsigned wav_get_num_bytes_per_frame(const wav_header *s);

int wav_get_num_frames(const wav_header *s);

long wav_get_frame_start(const wav_header *s, unsigned frame_number, uint32_t wavheader_size);

#endif // WAV_UTILS_H
