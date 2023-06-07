// This file is generated. Do not edit.
// Generated on: 07.06.2023 12:31:17

#ifndef model_GEN_H
#define model_GEN_H

#include "tensorflow/lite/c/common.h"

// Sets up the model with init and prepare steps.
TfLiteStatus model_init(void *flash_data = nullptr);
// Returns the input tensor with the given index.
TfLiteTensor *model_input(int index);
// Returns the output tensor with the given index.
TfLiteTensor *model_output(int index);
// Runs inference for the model.
TfLiteStatus model_invoke();

// Returns the number of input tensors.
inline size_t model_inputs() {
  return 1;
}
// Returns the number of output tensors.
inline size_t model_outputs() {
  return 1;
}

inline void *model_input_ptr(int index) {
  return model_input(index)->data.data;
}
inline size_t model_input_size(int index) {
  return model_input(index)->bytes;
}
inline int model_input_dims_len(int index) {
  return model_input(index)->dims->data[0];
}
inline int *model_input_dims(int index) {
  return &model_input(index)->dims->data[1];
}

inline void *model_output_ptr(int index) {
  return model_output(index)->data.data;
}
inline size_t model_output_size(int index) {
  return model_output(index)->bytes;
}
inline int model_output_dims_len(int index) {
  return model_output(index)->dims->data[0];
}
inline int *model_output_dims(int index) {
  return &model_output(index)->dims->data[1];
}

#endif
