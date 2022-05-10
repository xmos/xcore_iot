// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "xcore_custom_options.h"
#include "xcore_interpreter.h"

#include "FlashLoader.hpp"

namespace tflite {
namespace ops {
namespace micro {
namespace xcore {
namespace flash {
namespace rtos {

struct FlashOpData
    : XCoreOpData { // Inherits the operator name field from XCoreOpData
  uint32_t addr;
  size_t size;
  void *flash_data;
};

void *Init(TfLiteContext *context, const char *buffer, size_t length) {
  TFLITE_DCHECK(buffer != nullptr);

  auto op_data = construct_persistent_object<FlashOpData>(context);

  auto parser = CustomOptionParser(buffer, length);
  op_data->addr = parser.parseNamedCustomOption("addr").AsInt32();
  op_data->size = parser.parseNamedCustomOption("size").AsInt32();
  tflite::micro::xcore::XCoreInterpreter *xint =
      reinterpret_cast<tflite::micro::xcore::XCoreInterpreter *>(
          context->impl_);
  op_data->flash_data = xint->flash_data;
  op_data->name = "XC_Load_Flash";
  return op_data;
}

// Does all the requests for scratches
TfLiteStatus Prepare(TfLiteContext *context, TfLiteNode *node) {
  return kTfLiteOk;
}

TfLiteStatus Eval(TfLiteContext *context, TfLiteNode *node) {
  TfLiteEvalTensor *output = tflite::micro::GetEvalOutput(context, node, 0);
  int8_t *data_ptr = (int8_t *)tflite::micro::GetTensorData<int8_t>(output);
  auto op_data = reinterpret_cast<FlashOpData *>(node->user_data);

  auto *flash_loader =
      reinterpret_cast<::xcore::rtos::FlashLoader *>(op_data->flash_data);
  flash_loader->Load(data_ptr, op_data->addr, op_data->size);

  return kTfLiteOk;
}

} // namespace rtos
} // namespace flash

namespace rtos {

TfLiteRegistration *Register_LoadFromFlash() {
  static TfLiteRegistration r = {flash::rtos::Init, nullptr,
                                 flash::rtos::Prepare, flash::rtos::Eval};
  return &r;
}

} // namespace rtos

} // namespace xcore
} // namespace micro
} // namespace ops
} // namespace tflite