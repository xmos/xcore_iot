// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef XSCOPE_FILEIO_TASK_H_
#define XSCOPE_FILEIO_TASK_H_

#include <stdint.h>

void xscope_fileio_tasks_create(unsigned priority, void* app_data);

/* Signal to fileio that the application is done and files can be closed */
void xscope_fileio_user_done(void);

/* Send len_bytes
 * returns number of bytes sent */
size_t xscope_fileio_tx_to_host(uint8_t *buf, size_t len_bytes);

size_t xscope_fileio_rx_from_host(void *input_app_data, int8_t **input_data_frame, size_t frame_count);

#endif /* XSCOPE_FILEIO_TASK_H_ */
