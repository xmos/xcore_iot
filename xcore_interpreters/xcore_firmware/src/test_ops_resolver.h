// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef TEST_OPS_RESOLVER_H_
#define TEST_OPS_RESOLVER_H_

#include "tensorflow/lite/micro/compatibility.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"

namespace tflite {

// The magic number in the template parameter is the maximum number of ops that
// can be added to TestOpsResolver.
class TestOpsResolver : public MicroMutableOpResolver<128> {
 public:
  TestOpsResolver();

 private:
  TF_LITE_REMOVE_VIRTUAL_DELETE
};

}  // namespace tflite

#endif  // TEST_OPS_RESOLVER_H_
