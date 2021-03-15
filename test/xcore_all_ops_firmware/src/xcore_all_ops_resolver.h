// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef XCORE_ALL_OPS_RESOLVER_H_
#define XCORE_ALL_OPS_RESOLVER_H_

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/compatibility.h"
#include "tensorflow/lite/micro/kernels/xcore/xcore_ops.h"

namespace tflite {

// The magic number in the template parameter is the maximum number of ops that
// can be added to TestOpsResolver.
class XCoreAllOpsResolver : public AllOpsResolver {
 public:
  XCoreAllOpsResolver() : AllOpsResolver() {
    // Please keep this list of Custom Operators in alphabetical order.
    AddCustom(tflite::ops::micro::xcore::Add_8_OpCode,
              tflite::ops::micro::xcore::Register_Add_8());
    AddCustom(tflite::ops::micro::xcore::AvgPool2D_OpCode,
              tflite::ops::micro::xcore::Register_AvgPool2D());
    AddCustom(tflite::ops::micro::xcore::AvgPool2D_Global_OpCode,
              tflite::ops::micro::xcore::Register_AvgPool2D_Global());
    AddCustom(tflite::ops::micro::xcore::BConv2d_Bitpacked_DeepIn_OpCode,
              tflite::ops::micro::xcore::Register_BConv2D_Bitpacked_Deepin());
    AddCustom(tflite::ops::micro::xcore::BConv2d_Bitpacked_OpCode,
              tflite::ops::micro::xcore::Register_BConv2D_Bitpacked());
    AddCustom(tflite::ops::micro::xcore::BConv2d_Int8_DeepIn_DeepOut_OpCode,
              tflite::ops::micro::xcore::Register_BConv2D_Int8_Deepin_Deepout());
    AddCustom(tflite::ops::micro::xcore::BConv2d_Int8_OpCode,
              tflite::ops::micro::xcore::Register_BConv2D_Int8());
    AddCustom(tflite::ops::micro::xcore::Bsign_8_OpCode,
              tflite::ops::micro::xcore::Register_BSign_8());
    AddCustom(tflite::ops::micro::xcore::Conv2D_1x1_OpCode,
              tflite::ops::micro::xcore::Register_Conv2D_1x1());
    AddCustom(tflite::ops::micro::xcore::Conv2D_Deep_OpCode,
              tflite::ops::micro::xcore::Register_Conv2D_Deep());
    AddCustom(tflite::ops::micro::xcore::Conv2D_Depthwise_OpCode,
              tflite::ops::micro::xcore::Register_Conv2D_Depthwise());
    AddCustom(tflite::ops::micro::xcore::Conv2D_Shallow_OpCode,
              tflite::ops::micro::xcore::Register_Conv2D_Shallow());
    AddCustom(tflite::ops::micro::xcore::FullyConnected_8_OpCode,
              tflite::ops::micro::xcore::Register_FullyConnected_8());
    AddCustom(tflite::ops::micro::xcore::Lookup_8_OpCode,
              tflite::ops::micro::xcore::Register_Lookup_8());
    AddCustom(tflite::ops::micro::xcore::MaxPool2D_OpCode,
              tflite::ops::micro::xcore::Register_MaxPool2D());
    AddCustom(tflite::ops::micro::xcore::Pad_OpCode,
              tflite::ops::micro::xcore::Register_Pad());
  }

 private:
  TF_LITE_REMOVE_VIRTUAL_DELETE
};

}  // namespace tflite

#endif  // XCORE_ALL_OPS_RESOLVER_H_
