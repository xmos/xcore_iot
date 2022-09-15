// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef LINK_HELPERS_H_
#define LINK_HELPERS_H_

void link_disable(unsigned tileid, unsigned link_num);
void link_enable(unsigned tileid, unsigned link_num);
void link_reset(unsigned tileid, unsigned link_num);
void link_hello(unsigned tileid, unsigned link_num);
unsigned link_got_credit(unsigned tileid, unsigned link_num);

#endif /* LINK_HELPERS_H_ */