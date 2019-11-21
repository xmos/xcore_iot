/*
 * rtos_utils.h
 *
 *  Created on: Nov 20, 2019
 *      Author: mbruno
 */


#ifndef RTOS_UTILS_H_
#define RTOS_UTILS_H_

#define RTOS_MEMORY_BARRIER() asm volatile( "" ::: "memory" )

#if __XC__
extern "C" {
#endif //__XC__

/**
 * The RTOS must call this once for each core it
 * starts its scheduler on.
 *
 * \returns the ID of the core it is called on
 */
int rtos_core_register(void);

/**
 * Returns the ID of the calling core.
 *
 * rtos_core_register() must have been previously
 * called on the calling core.
 *
 * \returns the ID of the calling core.
 */
int rtos_core_id_get(void);

/**
 * Translates an RTOS core ID into the logical "xcore"
 * core ID.
 *
 * \param core_id An RTOS core ID
 *
 * \returns the logical "xcore" core ID
 */
int rtos_logical_core_id_get(int core_id);

/**
 * Returns the number of cores the RTOS is currently
 * running on.
 *
 * \returns the number of cores the RTOS is running on.
 */
int rtos_core_count(void);

#ifdef __XC__
}
#endif //__XC__

#endif /* RTOS_UTILS_H_ */
