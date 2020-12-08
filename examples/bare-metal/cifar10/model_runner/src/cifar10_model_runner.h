// This is a TensorFlow Lite model runner interface that has been
// generated using the generate_model_runner tool.

#ifndef CIFAR10_MODEL_RUNNER_H_
#define CIFAR10_MODEL_RUNNER_H_

#include "drivers/sw_services/model_runner/api/model_runner.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Create a new model runner for the model content specified.
 *
 * @param[out] ctx             Model runner context
 * @param[in]  model_content   Array containing model content
 */
ModelRunnerStatus cifar10_model_runner_create(model_runner_t *ctx, const uint8_t* model_content);

#ifdef __cplusplus
};
#endif

#endif  // CIFAR10_MODEL_RUNNER_H_