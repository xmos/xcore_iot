// This is a TensorFlow Lite model runner interface that has been
// generated using the generate_model_runner tool.

#ifndef {include_guard}
#define {include_guard}

#include <stddef.h>
#include <stdint.h>

typedef struct model_runner_struct model_runner_t;

struct model_runner_struct {{
    void *handle;
}};

#ifdef __cplusplus
extern "C" {{
#endif

void model_runner_init(model_runner_t *ctx, const uint8_t* model_content, uint8_t* arena, int arena_size);

int8_t* model_runner_get_input(model_runner_t *ctx);
size_t model_runner_get_input_size(model_runner_t *ctx);
void model_runner_get_input_quant(model_runner_t *ctx, float *scale, int* zero_point);

void model_runner_invoke(model_runner_t *ctx);

int8_t* model_runner_get_output(model_runner_t *ctx);
size_t model_runner_get_output_size(model_runner_t *ctx);
void model_runner_get_ouput_quant(float *scale, int* zero_point);

#ifdef __cplusplus
}};
#endif

#endif  // {include_guard}
