// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef AI_DEV_CTRL_H_
#define AI_DEV_CTRL_H_

typedef struct {
	unsigned offset_from_start;
	unsigned length;
} ai_dev_cmd_t;

#define AI_DEV_SETUP                    0x01
#define AI_DEV_INVOKE                   0x02
#define AI_DEV_SET_INPUT_TENSOR         0x03

#endif /* AI_DEV_CTRL_H_ */
