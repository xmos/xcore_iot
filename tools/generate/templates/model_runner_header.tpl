// This is a TensorFlow Lite model runner interface that has been
// generated using the generate_model_runner tool.

#ifndef {include_guard}
#define {include_guard}

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {{
#endif

void model_runner_initialize(const uint8_t* model_content, uint8_t* arena, int arena_size);

int8_t* model_runner_get_input();
size_t model_runner_get_input_size();
void model_runner_get_input_quantization(float *scale, int* zero_point);

void model_runner_invoke();

int8_t* model_runner_get_output();
size_t model_runner_get_output_size();
void model_runner_get_ouput_quantization(float *scale, int* zero_point);

#ifdef __cplusplus
}};
#endif

#endif  // {include_guard}
