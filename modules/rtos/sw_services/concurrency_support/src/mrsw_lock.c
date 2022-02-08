// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT MRSW_LOCK

#include <xcore/assert.h>

#include "rtos_osal.h"
#include "rtos_printf.h"
#include "concurrency_support.h"

#define MRSW_FLAG 1

rtos_osal_status_t mrsw_lock_create(mrsw_lock_t *ctx, char *name, mrsw_lock_type_t type)
{
    rtos_osal_status_t retval = RTOS_OSAL_SUCCESS;
    ctx->type = type;

    switch(type) {
        case MRSW_READER_PREFERRED:
        {
            read_pref_mrsw_lock_t *lock = rtos_osal_malloc(sizeof(read_pref_mrsw_lock_t));
            if (lock != NULL) {
                if (rtos_osal_semaphore_create(&lock->lock_global, name, 1, 1) != RTOS_OSAL_SUCCESS) {
                    rtos_osal_free(lock);
                    retval = RTOS_OSAL_ERROR;
                    break;
                }

                if (rtos_osal_mutex_create(&lock->lock_readers, name, RTOS_OSAL_NOT_RECURSIVE) != RTOS_OSAL_SUCCESS) {
                    rtos_osal_free(lock);
                    retval = RTOS_OSAL_ERROR;
                    break;
                }
                lock->num_readers_active = 0;

                ctx->lock_setup = lock;
            } else {
                retval = RTOS_OSAL_ERROR;
            }
            break;
        }
        case MRSW_WRITER_PREFERRED:
        {
            write_pref_mrsw_lock_t *lock = rtos_osal_malloc(sizeof(write_pref_mrsw_lock_t));
            if (lock != NULL) {
                if (rtos_osal_mutex_create(&lock->lock_global, name, RTOS_OSAL_NOT_RECURSIVE) != RTOS_OSAL_SUCCESS) {
                    rtos_osal_free(lock);
                    retval = RTOS_OSAL_ERROR;
                    break;
                }
                if (rtos_osal_event_group_create(&lock->cond, name) != RTOS_OSAL_SUCCESS) {
                    rtos_osal_mutex_delete(&lock->lock_global);
                    rtos_osal_free(lock);
                    retval = RTOS_OSAL_ERROR;
                    break;
                }
                lock->num_readers_active = 0;
                lock->num_writers_waiting = 0;
                lock->writer_active = 0;

                ctx->lock_setup = lock;
            } else {
                retval = RTOS_OSAL_ERROR;
            }
            break;
        }
        case MRSW_COUNT:
        default:
            retval = RTOS_OSAL_ERROR;
            break;
    }

    return retval;
}

rtos_osal_status_t mrsw_lock_delete(mrsw_lock_t *ctx)
{
    rtos_osal_status_t retval = RTOS_OSAL_SUCCESS;

    switch(ctx->type)
    {
        case MRSW_READER_PREFERRED:
        {
            read_pref_mrsw_lock_t *lock = (read_pref_mrsw_lock_t*)ctx->lock_setup;
            rtos_osal_semaphore_delete(&lock->lock_global);
            rtos_osal_mutex_delete(&lock->lock_readers);
            rtos_osal_free(lock);
            break;
        }
        case MRSW_WRITER_PREFERRED:
        {
            write_pref_mrsw_lock_t *lock = (write_pref_mrsw_lock_t*)ctx->lock_setup;
            rtos_osal_mutex_delete(&lock->lock_global);
            rtos_osal_event_group_delete(&lock->cond);
            rtos_osal_free(lock);
            break;
        }
        case MRSW_COUNT:
        default:
            retval = RTOS_OSAL_ERROR;
            break;
    }

    return retval;
}

rtos_osal_status_t mrsw_lock_reader_get(mrsw_lock_t *ctx, unsigned timeout)
{
    rtos_osal_status_t retval = RTOS_OSAL_SUCCESS;

    switch(ctx->type)
    {
        case MRSW_READER_PREFERRED:
        {
            read_pref_mrsw_lock_t *lock = (read_pref_mrsw_lock_t*)ctx->lock_setup;
            retval = rtos_osal_mutex_get(&lock->lock_readers, timeout);

            if (retval != RTOS_OSAL_SUCCESS) {
                rtos_printf("mrsw_lock_reader_get reader lock timeout\n");
                break;
            } else {
                int state = rtos_osal_critical_enter();
                lock->num_readers_active += 1;
                if (lock->num_readers_active == 1) {
                    rtos_osal_critical_exit(state);
                    retval = rtos_osal_semaphore_get(&lock->lock_global, RTOS_OSAL_PORT_WAIT_FOREVER);

                    if (retval != RTOS_OSAL_SUCCESS) {
                        xassert(0);
                        break;
                    }
                } else {
                    rtos_osal_critical_exit(state);
                }
                retval = rtos_osal_mutex_put(&lock->lock_readers);
            }
            break;
        }
        case MRSW_WRITER_PREFERRED:
        {
            write_pref_mrsw_lock_t *lock = (write_pref_mrsw_lock_t*)ctx->lock_setup;
            uint32_t tmp = 0;
            while(1) {
                rtos_printf("read get global lock\n");
                rtos_osal_mutex_get(&lock->lock_global, RTOS_OSAL_WAIT_FOREVER);
                rtos_printf("read got global lock\n");

                int state = rtos_osal_critical_enter();
                if ((!lock->writer_active) && (lock->num_writers_waiting == 0)) {
                    lock->num_readers_active += 1;
                    rtos_osal_critical_exit(state);
                    rtos_osal_mutex_put(&lock->lock_global);
                    break;
                } else {
                    rtos_osal_critical_exit(state);
                }

                rtos_printf("read get global lock 2\n");
                rtos_osal_mutex_put(&lock->lock_global);
                rtos_printf("read got global lock 2\n");
                if (RTOS_OSAL_TIMEOUT == rtos_osal_event_group_get_bits(
                                                &lock->cond,
                                                MRSW_FLAG,    /* req */
                                                RTOS_OSAL_PORT_CLEAR,
                                                &tmp,         /* actual */
                                                timeout)) {
                    /* We are giving up on reading */
                    rtos_printf("read get global lock 3\n");
                    rtos_osal_mutex_get(&lock->lock_global, RTOS_OSAL_WAIT_FOREVER);
                    rtos_printf("read got global lock 3\n");
                    state = rtos_osal_critical_enter();
                    {
                        lock->num_readers_active -= 1;
                    }
                    rtos_osal_critical_exit(state);
                    rtos_osal_mutex_put(&lock->lock_global);
                    break;
                }
            }
            break;
        }
        case MRSW_COUNT:
        default:
            retval = RTOS_OSAL_ERROR;
            break;
    }

    return retval;
}

rtos_osal_status_t mrsw_lock_reader_put(mrsw_lock_t *ctx)
{
    rtos_osal_status_t retval = RTOS_OSAL_SUCCESS;

    switch(ctx->type)
    {
        case MRSW_READER_PREFERRED:
        {
            read_pref_mrsw_lock_t *lock = (read_pref_mrsw_lock_t*)ctx->lock_setup;
            retval = rtos_osal_mutex_get(&lock->lock_readers, RTOS_OSAL_PORT_WAIT_FOREVER);

            if (retval != RTOS_OSAL_SUCCESS) {
                break;
            } else {
                int state = rtos_osal_critical_enter();
                xassert(lock->num_readers_active >= 1);
                lock->num_readers_active -= 1;
                if (lock->num_readers_active == 0) {
                    rtos_osal_critical_exit(state);
                    retval = rtos_osal_semaphore_put(&lock->lock_global);

                    if (retval != RTOS_OSAL_SUCCESS) {
                        break;
                    }
                } else {
                    rtos_osal_critical_exit(state);
                }
                retval = rtos_osal_mutex_put(&lock->lock_readers);
            }
            break;
        }
        case MRSW_WRITER_PREFERRED:
        {
            write_pref_mrsw_lock_t *lock = (write_pref_mrsw_lock_t*)ctx->lock_setup;
            rtos_printf("read put get global lock\n");
            retval = rtos_osal_mutex_get(&lock->lock_global, RTOS_OSAL_PORT_WAIT_FOREVER);
            rtos_printf("read put got global lock\n");

            if (retval != RTOS_OSAL_SUCCESS) {
                break;
            } else {
                int state = rtos_osal_critical_enter();
                lock->num_readers_active -= 1;
                if (lock->num_readers_active == 0) {
                    rtos_osal_critical_exit(state);
                    rtos_osal_event_group_set_bits(&lock->cond, MRSW_FLAG);
                } else {
                    rtos_osal_critical_exit(state);
                }
                retval = rtos_osal_mutex_put(&lock->lock_global);
            }
            break;
        }
        case MRSW_COUNT:
        default:
            retval = RTOS_OSAL_ERROR;
            break;
    }

    return retval;
}

rtos_osal_status_t mrsw_lock_writer_get(mrsw_lock_t *ctx, unsigned timeout)
{
    rtos_osal_status_t retval = RTOS_OSAL_SUCCESS;

    switch(ctx->type)
    {
        case MRSW_READER_PREFERRED:
        {
            read_pref_mrsw_lock_t *lock = (read_pref_mrsw_lock_t*)ctx->lock_setup;
            retval = rtos_osal_semaphore_get(&lock->lock_global, timeout);
            break;
        }
        case MRSW_WRITER_PREFERRED:
        {
            write_pref_mrsw_lock_t *lock = (write_pref_mrsw_lock_t*)ctx->lock_setup;
            uint32_t tmp = 0;
            rtos_printf("write get get global lock\n");
            rtos_osal_mutex_get(&lock->lock_global, RTOS_OSAL_WAIT_FOREVER);
            rtos_printf("write get got global lock\n");

            int state = rtos_osal_critical_enter();
            lock->num_writers_waiting += 1;
            rtos_printf("actives %d\n", lock->num_readers_active);
            if ((lock->num_readers_active > 0) || (lock->writer_active)) {
                rtos_osal_critical_exit(state);
                rtos_osal_mutex_put(&lock->lock_global);
                if (RTOS_OSAL_SUCCESS == rtos_osal_event_group_get_bits(
                                                &lock->cond,
                                                MRSW_FLAG,    /* req */
                                                RTOS_OSAL_PORT_CLEAR,
                                                &tmp,         /* actual */
                                                timeout)) {
                    rtos_printf("write get get global lock 2\n");
                    rtos_osal_mutex_get(&lock->lock_global, RTOS_OSAL_WAIT_FOREVER);
                    rtos_printf("write get got global lock 2\n");

                    state = rtos_osal_critical_enter();
                    {
                        lock->num_writers_waiting -= 1;
                        lock->writer_active = 1;
                    }
                    rtos_osal_critical_exit(state);
                    rtos_osal_mutex_put(&lock->lock_global);
                } else {
                    /* We are giving up on writing */
                    rtos_printf("write get get global lock 3\n");
                    rtos_osal_mutex_get(&lock->lock_global, RTOS_OSAL_WAIT_FOREVER);
                    rtos_printf("write get got global lock 3\n");

                    state = rtos_osal_critical_enter();
                    {
                        lock->num_writers_waiting -= 1;
                    }
                    rtos_osal_critical_exit(state);
                    rtos_osal_mutex_put(&lock->lock_global);
                }
            } else {
                lock->num_writers_waiting -= 1;
                lock->writer_active = 1;
                rtos_osal_critical_exit(state);
                rtos_osal_mutex_put(&lock->lock_global);
            }
            break;
        }
        case MRSW_COUNT:
        default:
            retval = RTOS_OSAL_ERROR;
            break;
    }

    return retval;
}

rtos_osal_status_t mrsw_lock_writer_put(mrsw_lock_t *ctx)
{
    rtos_osal_status_t retval = RTOS_OSAL_SUCCESS;

    switch(ctx->type)
    {
        case MRSW_READER_PREFERRED:
        {
            read_pref_mrsw_lock_t *lock = (read_pref_mrsw_lock_t*)ctx->lock_setup;
            retval = rtos_osal_semaphore_put(&lock->lock_global);
            break;
        }
        case MRSW_WRITER_PREFERRED:
        {
            write_pref_mrsw_lock_t *lock = (write_pref_mrsw_lock_t*)ctx->lock_setup;
            rtos_printf("write put get global lock\n");
            retval = rtos_osal_mutex_get(&lock->lock_global, RTOS_OSAL_PORT_WAIT_FOREVER);
            rtos_printf("write put got global lock\n");

            if (retval != RTOS_OSAL_SUCCESS) {
                break;
            } else {
                lock->writer_active = 0;
                rtos_osal_event_group_set_bits(&lock->cond, MRSW_FLAG);
                retval = rtos_osal_mutex_put(&lock->lock_global);
            }
            break;
        }
        case MRSW_COUNT:
        default:
            retval = RTOS_OSAL_ERROR;
            break;
    }

    return retval;
}
