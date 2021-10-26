// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef TILE_SUPPORT_H_
#define TILE_SUPPORT_H_

#include <xcore/chanend.h>

#include "i2c.h"
#include "i2s.h"

typedef struct tile_struct tile_ctx_t;

struct tile_struct {
    chanend_t c_other_tile;

    void *local_ctx;
};

extern tile_ctx_t *tile0_ctx;
extern tile_ctx_t *tile1_ctx;

typedef struct tile0_struct tile0_ctx_t;
struct tile0_struct {
    i2c_master_t i2c_ctx;
};

typedef struct tile1_struct tile1_ctx_t;
struct tile1_struct {
    port_t p_pdm_mic;
    int pdm_decimation_factor;
};

extern tile0_ctx_t *l_tile0_ctx;
extern tile1_ctx_t *l_tile1_ctx;

#endif /* TILE_SUPPORT_H_ */
