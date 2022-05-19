#define UNBUFFERED 0
#define BUFFERED 1

#ifndef TEST_BUFFER
#define TEST_BUFFER UNBUFFERED
#endif

#ifndef TEST_BAUD
#define TEST_BAUD 921600
#endif
#ifndef TEST_DATA_BITS
#define TEST_DATA_BITS 8
#endif
#ifndef TEST_PARITY
#define TEST_PARITY UART_PARITY_NONE
#endif
#ifndef TEST_STOP_BITS
#define TEST_STOP_BITS 1
#endif

#define SETSR(c) asm volatile("setsr %0" : : "n"(c));