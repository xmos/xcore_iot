// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef BENCHMARK_CODE_H_
#define BENCHMARK_CODE_H_

int unrolled_loop(void);
void benchmark_code(void);
void benchmark_random_data_throughput(void);
void benchmark_sequential_data_throughput_with_prefetch(void);
void benchmark_sequential_data_throughput(void);

#endif // BENCHMARK_CODE_H_
