// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


#include "FreeRTOS.h"

#include "wav_utils.h"


#define RIFF_SECTION_SIZE (12)
#define FMT_SUBCHUNK_MIN_SIZE (24)
#define EXTENDED_FMT_GUID_SIZE (16)
static const char wav_default_header[WAV_HEADER_BYTES] = {
        0x52, 0x49, 0x46, 0x46,
        0x00, 0x00, 0x00, 0x00,
        0x57, 0x41, 0x56, 0x45,
        0x66, 0x6d, 0x74, 0x20,
        0x10, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x64, 0x61, 0x74, 0x61,
        0x00, 0x00, 0x00, 0x00,
};

int get_wav_header_details(xscope_file_t *input_file, wav_header *s, unsigned *header_size){
  //Assume file is already open here. First rewind.
  xscope_fseek(input_file, 0, SEEK_SET);
  //read riff header section (12 bytes)
  xscope_fread(input_file, (uint8_t*)(&s->riff_header[0]), RIFF_SECTION_SIZE);
  if(memcmp(s->riff_header, "RIFF", sizeof(s->riff_header)) != 0)
  {
    rtos_printf("Error: couldn't find RIFF: 0x%x, 0x%x, 0x%x, 0x%x\n", s->riff_header[0], s->riff_header[1], s->riff_header[2], s->riff_header[3]);
    return 1;
  }

  if(memcmp(s->wave_header, "WAVE", sizeof(s->wave_header)) != 0)
  {
    rtos_printf("Error: couldn't find WAVE:, 0x%x, 0x%x, 0x%x, 0x%x\n", s->wave_header[0], s->wave_header[1], s->wave_header[2], s->wave_header[3]);
    return 1;
  }
  
  xscope_fread(input_file, (uint8_t*)&s->fmt_header[0], FMT_SUBCHUNK_MIN_SIZE);
  if(memcmp(s->fmt_header, "fmt ", sizeof(s->fmt_header)) != 0)
  {
    rtos_printf("Error: couldn't find fmt: 0x%x, 0x%x, 0x%x, 0x%x\n", s->fmt_header[0], s->fmt_header[1], s->fmt_header[2], s->fmt_header[3]);
    return 1;
  }
  
  unsigned fmt_subchunk_actual_size = s->fmt_chunk_size + sizeof(s->fmt_header) + sizeof(s->fmt_chunk_size); //fmt_chunk_size doesn't include the fmt_header(4) and size(4) bytes
  unsigned fmt_subchunk_remaining_size = fmt_subchunk_actual_size - FMT_SUBCHUNK_MIN_SIZE;
  
  if(s->audio_format == (short)0xfffe)
  {
    //seek to the end of fmt subchunk and rewind 16bytes to the beginning of GUID
    xscope_fseek(input_file, fmt_subchunk_remaining_size - EXTENDED_FMT_GUID_SIZE, SEEK_CUR);
    //The first 2 bytes of GUID is the audio_format.
    xscope_fread(input_file, (uint8_t *)&s->audio_format, sizeof(s->audio_format));
    //skip the rest of GUID
    xscope_fseek(input_file, EXTENDED_FMT_GUID_SIZE - sizeof(s->audio_format), SEEK_CUR);
  }
  else
  {
    //go to the end of fmt subchunk
    xscope_fseek(input_file, fmt_subchunk_remaining_size, SEEK_CUR);
  }
  if(s->audio_format != 1)
  {
    rtos_printf("Error: audio format(%d) is not PCM\n", s->audio_format);
    return 1;
  }
  
  //read header (4 bytes) for the next subchunk
  xscope_fread(input_file, (uint8_t*)&s->data_header[0], sizeof(s->data_header));
  //if next subchunk is fact, read subchunk size and skip it
  if(memcmp(s->data_header, "fact", sizeof(s->data_header)) == 0)
  {
    uint32_t chunksize;
    xscope_fread(input_file, (uint8_t *)&chunksize, sizeof(s->data_bytes));
    xscope_fseek(input_file, chunksize, SEEK_CUR);
    xscope_fread(input_file, (uint8_t*)(&s->data_header[0]), sizeof(s->data_header));
  }
  //only thing expected at this point is the 'data' subchunk. Throw error if not found.
  if(memcmp(s->data_header, "data", sizeof(s->data_header)) != 0)
  {
    rtos_printf("Error: couldn't find data: 0x%x, 0x%x, 0x%x, 0x%x\n", s->data_header[0], s->data_header[1], s->data_header[2], s->data_header[3]);
    return 1;
  }
  //read data subchunk size. 
  xscope_fread(input_file, (uint8_t *)&s->data_bytes, sizeof(s->data_bytes));
  *header_size = xscope_ftell(input_file); //total file size should be header_size + data_bytes
  //No need to close file - handled by caller

  return 0;
}

int wav_form_header(wav_header *header,
        short audio_format,
        short num_channels,
        int sample_rate,
        short bit_depth,
        int num_frames){
    memcpy((char*)header, wav_default_header, WAV_HEADER_BYTES);

    header->audio_format = audio_format;
    header->num_channels = num_channels;
    header->sample_rate = sample_rate;
    header->bit_depth = bit_depth;

    header->byte_rate = sample_rate*bit_depth*num_channels/8;

    header->sample_alignment = num_channels* (bit_depth/8);
    int data_bytes = num_frames * num_channels * (bit_depth/8);
    header->data_bytes = data_bytes;
    header->wav_size = data_bytes + WAV_HEADER_BYTES - 8;

    return 0;
}

unsigned wav_get_num_bytes_per_frame(const wav_header *s){
    int bytes_per_sample = s->bit_depth/8;
    return (unsigned)(bytes_per_sample * s->num_channels);
}

int wav_get_num_frames(const wav_header *s){
    unsigned bytes_per_frame = wav_get_num_bytes_per_frame(s);
    return s->data_bytes / bytes_per_frame;
}

long wav_get_frame_start(const wav_header *s, unsigned frame_number, uint32_t wavheader_size){
    return wavheader_size + frame_number * wav_get_num_bytes_per_frame(s);
}
