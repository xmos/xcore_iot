// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef SDRAM_CONF_H_
#define SDRAM_CONF_H_

/* Buffer sizes */
#define SDRAMCONF_READ_BUFFER_SIZE      256
#define SDRAMCONF_WRITE_BUFFER_SIZE     256

/* Use the IS45S16160D 256Mb part or comparable */
#define SDRAMCONF_USE_256MB             1

/* Use IS42S16400D 64Mb part or comparable */
#define SDRAMCONF_USE_64MB              0

/* Use custom defined part */
#define SDRAMCONF_USE_CUSTOM            0


/* Default part parameters */
#if SDRAMCONF_USE_256MB
#define SDRAMCONF_CAS_LATENCY           2
#define SDRAMCONF_ROW_WORDS             256
#define SDRAMCONF_COL_BITS              16
#define SDRAMCONF_COL_ADDR_BITS         9
#define SDRAMCONF_ROW_ADDR_BITS         13
#define SDRAMCONF_BANK_ADDR_BITS        2
#define SDRAMCONF_REFRESH_MS            64
#define SDRAMCONF_REFRESH_CYCLES        8192
#define SDRAMCONF_CLOCK_DIVIDER         4
#endif

#if SDRAMCONF_USE_64MB
#define SDRAMCONF_CAS_LATENCY           2
#define SDRAMCONF_ROW_WORDS             128
#define SDRAMCONF_COL_BITS              16
#define SDRAMCONF_COL_ADDR_BITS         8
#define SDRAMCONF_ROW_ADDR_BITS         12
#define SDRAMCONF_BANK_ADDR_BITS        2
#define SDRAMCONF_REFRESH_MS            64
#define SDRAMCONF_REFRESH_CYCLES        4096
#define SDRAMCONF_CLOCK_DIVIDER         4
#endif

/* Verify config is valid */
#if ( SDRAMCONF_USE_256MB && SDRAMCONF_USE_64MB ) || ( SDRAMCONF_USE_CUSTOM && ( SDRAMCONF_USE_256MB || SDRAMCONF_USE_64MB ))
#error Only parameters for a single part can be specified
#endif

#if SDRAMCONF_USE_CUSTOM
#ifndef SDRAMCONF_CAS_LATENCY
#error SDRAMCONF_CAS_LATENCY must be defined
#endif
#ifndef SDRAMCONF_ROW_WORDS
#error SDRAMCONF_ROW_WORDS must be defined
#endif
#ifndef SDRAMCONF_COL_BITS
#error SDRAMCONF_COL_BITS must be defined
#endif
#ifndef SDRAMCONF_COL_ADDR_BITS
#error SDRAMCONF_COL_ADDR_BITS must be defined
#endif
#ifndef SDRAMCONF_ROW_ADDR_BITS
#error SDRAMCONF_ROW_ADDR_BITS must be defined
#endif
#ifndef SDRAMCONF_BANK_ADDR_BITS
#error SDRAMCONF_BANK_ADDR_BITS must be defined
#endif
#ifndef SDRAMCONF_REFRESH_MS
#error SDRAMCONF_REFRESH_MS must be defined
#endif
#ifndef SDRAMCONF_REFRESH_CYCLES
#error SDRAMCONF_REFRESH_CYCLES must be defined
#endif
#ifndef SDRAMCONF_CLOCK_DIVIDER
#error SDRAMCONF_CLOCK_DIVIDER must be defined
#endif
#endif

#endif /* SDRAM_CONF_H_ */
