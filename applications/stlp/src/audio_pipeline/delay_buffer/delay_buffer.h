#ifndef DELAY_BUFFER_H
#define DELAY_BUFFER_H

#define MAX_DELAY_BUF_CHANNELS (2)
#define DELAY_BUF_MAX_DELAY_MS                ( 150 )
#define DELAY_BUF_MAX_DELAY_SAMPLES           ( 16000*DELAY_BUF_MAX_DELAY_MS/1000 )

typedef struct {
    // Circular buffer to store the samples
    int32_t delay_buffer[MAX_DELAY_BUF_CHANNELS][DELAY_BUF_MAX_DELAY_SAMPLES];
    // index of the value for the samples to be stored in the buffer
    int32_t curr_idx[MAX_DELAY_BUF_CHANNELS];
    int32_t delay_samples;
} delay_buf_state_t;

void delay_buffer_init(delay_buf_state_t *state, int default_delay_samples);
void get_delayed_sample(delay_buf_state_t *delay_state, int32_t *sample, int32_t ch);
void update_delay_samples(delay_buf_state_t *delay_state, int32_t num_samples);
void reset_partial_delay_buffer(delay_buf_state_t *delay_state, int32_t ch);
#endif
