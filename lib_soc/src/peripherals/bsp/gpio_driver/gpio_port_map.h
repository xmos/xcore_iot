// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef GPIO_PORT_MAP_H_
#define GPIO_PORT_MAP_H_

#define GPIO_TOTAL_PORT_CNT   (29) //(32)

typedef enum {
    gpio_1A = 0,
    gpio_1B,
    gpio_1C,
    gpio_1D,
    gpio_1E,
    gpio_1F,
    gpio_1G,
    gpio_1H,
    gpio_1I,
    gpio_1J,
    gpio_1K,
    gpio_1L,
    gpio_1M,
    gpio_1N,
    gpio_1O,
    gpio_1P,
    gpio_4A,
    gpio_4B,
    gpio_4C,
    // gpio_4D,
    // gpio_4E,
    // gpio_4F,
    gpio_8A,
    gpio_8B,
    gpio_8C,
    gpio_8D,
    gpio_16A,
    gpio_16B,
    gpio_16C,
    gpio_16D,
    gpio_32A,
    gpio_32B
} gpio_id_t;

#endif /* GPIO_PORT_MAP_H_ */
