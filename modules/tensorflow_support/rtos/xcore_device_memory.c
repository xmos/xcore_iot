// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1
#include "xcore_device_memory.h"

#include <stddef.h>
#include <string.h>

#ifdef XCORE
#include <xcore/port.h>

#define WORDS_TO_BYTES(w) ((w) * sizeof(uint32_t))
#define BYTES_TO_WORDS(b) (((b) + sizeof(uint32_t) - 1) / sizeof(uint32_t))

#define WORD_TO_BYTE_ADDRESS(w) WORDS_TO_BYTES(w)
#define BYTE_TO_WORD_ADDRESS(b) ((b) / sizeof(uint32_t))

#if USE_SWMEM
#include <xcore/swmem_fill.h>

#include "rtos/drivers/qspi_flash/api/rtos_qspi_flash.h"
#include "rtos/drivers/swmem/api/rtos_swmem.h"

static rtos_qspi_flash_t *qspi_flash_ctx = NULL;

void rtos_swmem_read_request(unsigned offset, uint32_t *buf)
{
    rtos_printf("SwMem fill request for offset 0x%08x\n", offset);
    if(qspi_flash_ctx != NULL) {
        rtos_printf("read %d\n", rtos_core_id_get());
        rtos_qspi_flash_read(qspi_flash_ctx,
                             (uint8_t *)buf,
                             (unsigned)offset,
                             WORDS_TO_BYTES(SWMEM_FILL_SIZE_WORDS));
        rtos_printf("read done\n");
    }
}

void swmem_setup(rtos_qspi_flash_t *ctx, unsigned swmem_task_priority) {
    qspi_flash_ctx = ctx;
    rtos_swmem_init(RTOS_SWMEM_READ_FLAG);
    rtos_swmem_start(swmem_task_priority);
}
#endif /* USE_SWMEM */

void memload(void *dest, void *src, size_t size) {
#if USE_SWMEM
    if (IS_SWMEM(src)) {
        if(qspi_flash_ctx != NULL) {
            rtos_qspi_flash_read(qspi_flash_ctx,
                                 (uint8_t *)dest,
                                 (unsigned)size,
                                 size);
        }
    } else
#endif /* USE_SWMEM */
    if (IS_EXTMEM(src)) {
        memcpy(dest, src, size);
    }
}

#endif  // XCORE
