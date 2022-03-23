// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef MODEL_MEMORY_LOADER_H_
#define MODEL_MEMORY_LOADER_H_

#include <cstring>
#include <xs1.h>

#include "tensorflow/lite/micro/kernels/xcore/xcore_memory_loader.h"

extern "C" {
#include "nn_operator.h"
}

#define IS_RAM(a)                    \
  (((uintptr_t)a >= XS1_RAM_BASE) && \
   ((uintptr_t)a <= (XS1_RAM_BASE + XS1_RAM_SIZE)))
#define IS_SWMEM(a)                    \
  (((uintptr_t)a >= XS1_SWMEM_BASE) && \
   (((uintptr_t)a <= (XS1_SWMEM_BASE - 1 + XS1_SWMEM_SIZE))))

extern "C" {
size_t swmem_load(void *dest, const void *src, size_t size);
}

namespace tflite {
namespace micro {
namespace xcore {

class ModelMemoryLoader : public MemoryLoader {
 public:
  ModelMemoryLoader() {}

  size_t Load(void **dest, const void *src, size_t size) {
#ifdef USE_SWMEM
    if (IS_SWMEM(src)) {
      return swmem_load(*dest, src, size);
    } else
#endif /* USE_SWMEM */
    {
      if (IS_RAM(src)) {
        *dest = const_cast<void *>(src);
        return 0;
      } else if (size >= 128) {
        vpu_memcpy_ext(*dest, src, size);
      } else {
        memcpy(*dest, src, size);
      }
      return size;
    }
  }
};

}  // namespace xcore
}  // namespace micro
}  // namespace tflite

#endif  // MODEL_MEMORY_LOADER_H_
