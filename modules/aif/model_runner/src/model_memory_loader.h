// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef MODEL_MEMORY_LOADER_H_
#define MODEL_MEMORY_LOADER_H_

#include <cstring>

#include "tensorflow/lite/micro/kernels/xcore/xcore_memory_loader.h"

extern "C" {
#include "nn_operator.h"
}

#define IS_RAM(a) (((uintptr_t)a >= 0x80000) && ((uintptr_t)a <= 0x100000))
#define IS_SWMEM(a) \
  (((uintptr_t)a >= 0x40000000) && (((uintptr_t)a <= 0x80000000)))

extern "C" {
size_t swmem_load(void *dest, const void *src, size_t size);
}

namespace tflite {
namespace micro {
namespace xcore {

class ModelMemoryLoader : public MemoryLoader<ModelMemoryLoader> {
 public:
  MemoryLoader() {}

  size_t load_impl(void **dest, const void *src, size_t size);
};

size_t ModelMemoryLoader::load_impl(void **dest, const void *src, size_t size) {
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

}  // namespace xcore
}  // namespace micro
}  // namespace tflite

#endif  // MODEL_MEMORY_LOADER_H_
