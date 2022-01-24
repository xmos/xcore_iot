// Copyright 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef MRSW_LOCK_H_
#define MRSW_LOCK_H_

/**
 * \addtogroup multiple_reader_single_writer_lock multiple_reader_single_writer_lock
 *
 * The public API for using the multiple reader single writer lock implementation.
 * @{
 */


/*
 * Enumeration representing lock types
 */
typedef enum {
    MRSW_READER_PREFERRED = 0,
    MRSW_WRITER_PREFERRED,
    MRSW_COUNT
} mrsw_lock_type_t;

#include "rtos_osal.h"

/**
 * Struct representing an MRSW instance.
 *
 * The members in this struct should not be accessed directly.
 */
typedef struct mrsw_lock {
    void* lock_setup;
    mrsw_lock_type_t type;
} mrsw_lock_t;

/**
 * Struct representing an reader preferred MRSW
 *
 * The members in this struct should not be accessed directly.
 */
typedef struct read_pref_mrsw_lock {
    rtos_osal_semaphore_t lock_global;
    rtos_osal_mutex_t lock_readers;
    uint32_t num_readers_active;
} read_pref_mrsw_lock_t;

/**
 * Struct representing an writer preferred MRSW
 *
 * The members in this struct should not be accessed directly.
 */
typedef struct write_pref_mrsw_lock {
    rtos_osal_mutex_t lock_global;
    rtos_osal_event_group_t cond;
    uint32_t num_readers_active;
    uint32_t num_writers_waiting;
    uint32_t writer_active;
} write_pref_mrsw_lock_t;

/**
 * Create a MRSW lock
 *
 * \param ctx    A pointer to an uninitialized lock context
 * \param name   An optional ASCII name
 * \param type   The type of lock
 *
 * \returns      RTOS_OSAL_SUCCESS on success
 */
rtos_osal_status_t mrsw_lock_create(mrsw_lock_t *ctx, char *name, mrsw_lock_type_t type);

/**
 * Destroy a MRSW lock
 *
 * Note: This does not check if it is safe to delete locks
 *
 * \param ctx    A pointer to the associated lock context
 *
 * \returns      RTOS_OSAL_SUCCESS on success
 *               RTOS_OSAL_ERROR otherwise
 */
rtos_osal_status_t mrsw_lock_delete(mrsw_lock_t *ctx);

/**
 * \addtogroup multiple_reader_single_writer_lock_reader multiple_reader_single_writer_lock_reader
 *
 * The core functions for using an MRSW lock as a reader
 * @{
 */

/**
 * Attempt to acquire a lock as a reader.
 *
 * \param ctx     A pointer to the associated lock context
 * \param timeout A timeout before giving up
 *
 * \returns      RTOS_OSAL_SUCCESS on success
 *               RTOS_OSAL_TIMEOUT on timeout
 *               RTOS_OSAL_ERROR otherwise
 */
rtos_osal_status_t mrsw_lock_reader_get(mrsw_lock_t *ctx, unsigned timeout);

/**
 * Give an acquired lock as a reader.
 *
 * Note: User must not give a lock they do not own.
 *
 * \param ctx     A pointer to the associated lock context
 *
 * \returns      RTOS_OSAL_SUCCESS on success
 *               RTOS_OSAL_ERROR otherwise
 */
rtos_osal_status_t mrsw_lock_reader_put(mrsw_lock_t *ctx);

/**@}*/

/**
 * \addtogroup multiple_reader_single_writer_lock_writer multiple_reader_single_writer_lock_writer
 *
 * The core functions for using an MRSW lock as a writer
 * @{
 */

/**
 * Attempt to acquire a lock as a writer.
 *
 * \param ctx     A pointer to the associated lock context
 * \param timeout A timeout before giving up
 *
 * \returns      RTOS_OSAL_SUCCESS on success
 *               RTOS_OSAL_TIMEOUT on timeout
 *               RTOS_OSAL_ERROR otherwise
 */
rtos_osal_status_t mrsw_lock_writer_get(mrsw_lock_t *ctx, unsigned timeout);
/**
 * Give an acquired lock as a writer.
 *
 * Note: User must not give a lock they do not own.
 *
 * \param ctx     A pointer to the associated lock context
 *
 * \returns      RTOS_OSAL_SUCCESS on success
 *               RTOS_OSAL_ERROR otherwise
 */
rtos_osal_status_t mrsw_lock_writer_put(mrsw_lock_t *ctx);

/**@}*/

/**@}*/

#endif /* MRSW_LOCK_H_ */
