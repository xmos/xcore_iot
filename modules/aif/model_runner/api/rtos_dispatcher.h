// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef RTOS_DISPATCHER_H_
#define RTOS_DISPATCHER_H_

#include "dispatcher.h"
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
  RTOSDispatcher(dispatcher_t *dispatcher);
  ~RTOSDispatcher();

  TfLiteStatus Invoke(void **arguments, size_t size) const override;

private:
  dispatcher_t *dispatcher_;
  dispatch_group_t *group_;
};

} // namespace xcore
} // namespace micro
} // namespace tflite

#endif // RTOS_DISPATCHER_H_