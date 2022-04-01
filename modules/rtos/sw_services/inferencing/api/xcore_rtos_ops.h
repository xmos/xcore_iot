// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef XCORE_RTOS_OPS_H_
#define XCORE_RTOS_OPS_H_

#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "xcore_utils.h"

namespace tflite {
namespace ops {
namespace micro {
namespace xcore {
namespace rtos {

constexpr const char *Conv2D_V2_OpCode = "XC_conv2d_v2";

TfLiteRegistration *Register_Conv2D_V2();

} // namespace rtos
} // namespace xcore
} // namespace micro
} // namespace ops
} // namespace tflite

#endif // XCORE_RTOS_OPS_H_