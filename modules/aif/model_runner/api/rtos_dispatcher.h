// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef RTOS_DISPATCHER_H_
#define RTOS_DISPATCHER_H_

#include "dispatch.h"
#include "tensorflow/lite/micro/kernels/xcore/xcore_dispatcher.h"

namespace tflite {
namespace micro {
namespace xcore {

/**
 * RTOSDispatcher class
 *
 * RTOS implementation of the Dispatcher abstract base class.
 */
class RTOSDispatcher : public Dispatcher {
 public:
  RTOSDispatcher(dispatch_queue_t *dispatch_queue);
  ~RTOSDispatcher();

  TfLiteStatus Invoke(void **arguments, size_t size) const override;

 private:
  dispatch_queue_t *dispatch_queue_;
  dispatch_group_t *group_;
};

}  // namespace xcore
}  // namespace micro
}  // namespace tflite

#endif  // RTOS_DISPATCHER_H_