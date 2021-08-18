// This is a TensorFlow Lite model runner interface that has been
// generated using the generate_model_runner tool.

#ifndef HOTDOG_NOT_HOTDOG_MODEL_RUNNER_H_
#define HOTDOG_NOT_HOTDOG_MODEL_RUNNER_H_

#include "model_runner.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Create a hotdog_not_hotdog model runner.
 *
 * @param[out] ctx      Model runner context
 * @param[in]  buffer   Buffer for interpreter.  If NULL, will be allocated with
 *                      malloc.  The caller is responsible for freeing the memory
 *                      if needed.  To determine the size at run time call:
 *
 *       size_t size = model_runner_buffer_size_get();
 */
void hotdog_not_hotdog_model_runner_create(model_runner_t *ctx, void *buffer);

#ifdef __cplusplus
};
#endif

#endif  // HOTDOG_NOT_HOTDOG_MODEL_RUNNER_H_