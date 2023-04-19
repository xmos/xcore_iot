// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include <stdint.h>
#include <print.h>

// Helper to disassemble USB packets into 32b left aligned audio samples
static inline void unpack_buff_to_samples(uint8_t input[], const unsigned n_samples, const unsigned slot_size, int32_t output[]){
  switch(slot_size){
    case 4:
      for (int i = 0; i < n_samples; i++){
        unsigned base = i * 4;
        output[i] = ((uint32_t)input[base + 3] << 24) | ((uint32_t)input[base + 2] << 16) | ((uint32_t)input[base + 1] << 8) | (uint32_t)input[base + 0];
      }
    break;
    case 3:
      for (int i = 0; i < n_samples; i++){
        unsigned base = i * 3;
        output[i] = ((uint32_t)input[base + 2] << 24) | ((uint32_t)input[base + 1] << 16) | ((uint32_t)input[base + 0] << 8);
      }
    break;
    case 2:
      for (int i = 0; i < n_samples; i++){
        unsigned base = i * 2;
        output[i] = ((uint32_t)input[base + 1] << 24) | ((uint32_t)input[base + 0] << 16);
      }
    break;
    default:
      printstr("Invalid slot_size\n");
    break;
  }
}

// Helper to assemble USB packets from 32b left aligned audio samples
static inline void pack_samples_to_buff(int32_t input[], const unsigned n_samples, const unsigned slot_size, uint8_t output[]){
  switch(slot_size){
    case 4:
      for (int i = 0; i < n_samples; i++){
        unsigned base = i * 4;
        unsigned in_word = (unsigned)input[i];
        output[base + 0] = in_word & 0xff;
        output[base + 1] = (in_word & 0xff00) >> 8;
        output[base + 2] = (in_word & 0xff0000) >> 16;
        output[base + 3] = (in_word) >> 24;
      }
    break;
    case 3:
      for (int i = 0; i < n_samples; i++){
        unsigned base = i * 3;
        unsigned in_word = (unsigned)input[i];
        output[base + 0] = (in_word & 0xff00) >> 8;
        output[base + 1] = (in_word & 0xff0000) >> 16;
        output[base + 2] = (in_word) >> 24;
      }
    break;
    case 2:
      for (int i = 0; i < n_samples; i++){
        unsigned base = i * 2;
        unsigned in_word = (unsigned)input[i];
        output[base + 0] = (in_word & 0xff0000) >> 16;
        output[base + 1] = (in_word) >> 24;
      }
    break;
    default:
      printstr("Invalid slot_size\n");
    break;
  }
}
