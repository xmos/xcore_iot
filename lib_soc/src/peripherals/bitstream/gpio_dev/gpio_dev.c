// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include "xcore_c.h"

#include "gpio_dev.h"

void gpio_port_alloc(unsigned *p, int id)
{
    port_alloc((port*)p, (port_id_t)id);
}

void gpio_port_free( unsigned *p )
{
    port_free((port*)p);
}

void gpio_port_in( unsigned p, uint32_t *data )
{
    port_in((port)p, data);
}

void gpio_port_out(unsigned p, uint32_t data)
{
    port_out((port)p, data);
}

void gpio_port_peek( unsigned p, uint32_t *data )
{
    port_peek((port)p, data);
}
