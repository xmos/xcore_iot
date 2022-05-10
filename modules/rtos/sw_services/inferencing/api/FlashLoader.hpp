// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef RTOS_FLASH_LOADER_H_
#define RTOS_FLASH_LOADER_H_

extern "C" {
#include "ff.h"
#include "rtos_qspi_flash.h"
}

namespace xcore {
namespace rtos {

class FlashLoader {
public:
  FlashLoader() = default;
  virtual ~FlashLoader() = default;
  virtual void Load(void *dest, uint32_t addr, size_t n) const = 0;
};

class FatFSLoader final : public FlashLoader {
public:
  void Init(FIL *file) { file_ = file; }
  void Load(void *dest, uint32_t addr, size_t n) const override {
    size_t bytes_read;
    f_lseek(file_, addr);
    f_read(file_, dest, n, &bytes_read);
  }

private:
  FIL *file_;
};

class FlashDriverLoader final : public FlashLoader {
public:
  void Init(rtos_qspi_flash_t *ctx, size_t top) {
    ctx_ = ctx;
    top_ = top;
  }
  void Load(void *dest, uint32_t addr, size_t n) const override {
    rtos_qspi_flash_read(ctx_, static_cast<uint8_t *>(dest), addr - top_, n);
  }

private:
  rtos_qspi_flash_t *ctx_;
  size_t top_;
};

} // namespace rtos
} // namespace xcore

#endif // RTOS_FLASH_LOADER_H_