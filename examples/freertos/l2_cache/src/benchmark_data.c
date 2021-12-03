// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "benchmark_data.h"

#include "app_common.h"
#include "swmem_macros.h"

#define DATA_3(X)  ((X)+0),((X)+1),((X)+2),((X)+3),((X)+4),((X)+5),((X)+6),((X)+7)
#define DATA_5(X)  DATA_3((X)+0),DATA_3((X)+8),DATA_3((X)+16),DATA_3((X)+24)
#define DATA_7(X)  DATA_5((X)+0),DATA_5((X)+32),DATA_5((X)+64),DATA_5((X)+96)
#define DATA_9(X)  DATA_7((X)+0),DATA_7((X)+128),DATA_7((X)+256),DATA_7((X)+384)
#define DATA_11(X)  DATA_9((X)+0),DATA_9((X)+512),DATA_9((X)+1024),DATA_9((X)+1536)
#define DATA_13(X)  DATA_11((X)+0),DATA_11((X)+2048),DATA_11((X)+4096),DATA_11((X)+6144)
#define DATA_15(X)  DATA_13((X)+0),DATA_13((X)+8192),DATA_13((X)+16384),DATA_13((X)+24576)
#define DATA_16    DATA_15(0),DATA_15(32768)

XCORE_DATA_SECTION_ATTRIBUTE
WORD_ALIGNED
const int data_array[DATA_ARRAY_LEN] = { DATA_16 };

const int data_array_len = DATA_ARRAY_LEN;
const int data_array_size = DATA_ARRAY_LEN * sizeof(data_array[0]);
