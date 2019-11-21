/*
 * rtos_support.h
 *
 *  Created on: Nov 18, 2019
 *      Author: mbruno
 */


#ifndef RTOS_SUPPORT_H_
#define RTOS_SUPPORT_H_

/* The maximum number of cores an SMP RTOS may use */
#define RTOS_MAX_CORE_COUNT 8

/* Config file to be provided by the RTOS */
#include "rtos_support_rtos_config.h"

/* Library header files */
#include "rtos_utils.h"
#include "rtos_interrupt.h"

#ifndef __XC__
#include "rtos_irq.h"
#endif

#endif /* RTOS_SUPPORT_H_ */
