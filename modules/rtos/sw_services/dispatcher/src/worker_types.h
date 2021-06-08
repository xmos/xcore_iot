// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#ifndef WORKER_TYPES_H_
#define WORKER_TYPES_H_

typedef enum WorkerType {
  UninitializedWorker,
  ThreadWorker,
  ISRWorker
} WorkerType;

#endif // WORKER_TYPES_H_