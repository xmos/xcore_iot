// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef SL_WFX_HOST_H_
#define SL_WFX_HOST_H_

void sl_wfx_host_set_hif(soc_peripheral_t spi_dev,
                         soc_peripheral_t gpio_dev,
                         gpio_id_t wirq_gpio_port, int wirq_bit,
                         gpio_id_t wup_gpio_port, int wup_bit);

#endif /* SL_WFX_HOST_H_ */
