// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include "tile_support.h"

static tile_ctx_t tile0_ctx_s;
tile_ctx_t *tile0_ctx = &tile0_ctx_s;

static tile_ctx_t tile1_ctx_s;
tile_ctx_t *tile1_ctx = &tile1_ctx_s;

static tile0_ctx_t l_tile0_ctx_s;
tile0_ctx_t *l_tile0_ctx = &l_tile0_ctx_s;

static tile1_ctx_t l_tile1_ctx_s;
tile1_ctx_t *l_tile1_ctx = &l_tile1_ctx_s;
