// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include "test_ops_resolver.h"

#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/kernels/xcore/xcore_ops.h"

namespace tflite {

TestOpsResolver::TestOpsResolver() {
  // Please keep this list of Builtin Operators in alphabetical order.
  //
  // Several Ops that are supported by the TFLu runtime are commented out below.
  // This is to keep the code size of the runtime as small as possible while
  // still registering as many Ops as we think we may need for our testing.
  //
  // NOTE: If you uncomment or add Ops to the list below, you may need to reduce
  // the size of the tensor arena.  The link should fail if the target device
  // does not have enough RAM for the code+data+stack.  Just be aware that
  // adding Ops below has consequences and tradeoffs.
  AddAbs();
  AddAdd();
  AddArgMax();
  AddArgMin();
  AddAveragePool2D();
  AddCeil();
  AddConcatenation();
  AddConv2D();
  AddCos();
  AddDepthwiseConv2D();
  AddDequantize();
  AddEqual();
  AddFloor();
  AddFullyConnected();
  AddGreater();
  AddGreaterEqual();
  // AddHardSwish();
  AddL2Normalization();
  AddLess();
  AddLessEqual();
  AddLog();
  AddLogicalAnd();
  AddLogicalNot();
  AddLogicalOr();
  AddLogistic();
  AddMaximum();
  AddMaxPool2D();
  AddMean();
  AddMinimum();
  AddMul();
  AddNeg();
  AddNotEqual();
  AddPack();
  AddPad();
  AddPadV2();
  AddPrelu();
  AddQuantize();
  AddReduceMax();
  AddRelu();
  AddRelu6();
  AddReshape();
  // AddResizeNearestNeighbor();
  AddRound();
  // AddRsqrt();
  AddShape();
  AddSin();
  AddSoftmax();
  AddSplit();
  AddSplitV();
  AddSqrt();
  AddSquare();
  // AddStridedSlice();
  AddSub();
  // AddSvdf();
  AddTanh();
  // AddUnpack();

  AddCustom(tflite::ops::micro::xcore::AvgPool2D_OpCode,
            tflite::ops::micro::xcore::Register_AvgPool2D());
  AddCustom(tflite::ops::micro::xcore::AvgPool2D_Global_OpCode,
            tflite::ops::micro::xcore::Register_AvgPool2D_Global());
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
  AddCustom(tflite::ops::micro::xcore::Bsign_8_OpCode,
            tflite::ops::micro::xcore::Register_BSign_8());
  AddCustom(tflite::ops::micro::xcore::BConv2d_Bitpacked_DeepIn_OpCode,
            tflite::ops::micro::xcore::Register_BConv2D_Bitpacked_Deepin());
  AddCustom(tflite::ops::micro::xcore::BConv2d_Bitpacked_OpCode,
            tflite::ops::micro::xcore::Register_BConv2D_Bitpacked());
}

}  // namespace tflite
