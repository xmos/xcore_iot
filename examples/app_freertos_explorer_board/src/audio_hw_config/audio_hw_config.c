// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved

/* BSP/bitstream headers */
#include "DAC3101.h"
#include "i2c_driver.h"

/* Library headers */
#include "soc.h"

/* Application headers */

#define I2S_CHANS_DAC 2


static void initDAC3101(soc_peripheral_t i2c_dev)
{
    unsigned char data = 0;
	i2c_regop_res_t i2c_res;

	// Set register page to 0
	DAC3101_REGWRITE(DAC3101_PAGE_CTRL, 0x00);
	// Initiate SW reset (PLL is powered off as part of reset)
	DAC3101_REGWRITE(DAC3101_SW_RST, 0x01);
	delay_milliseconds(1);

	// We are supplying a 24.576MHz on mclk
	// PLL_CLK = 24.576MHz

	// CODEC_CLKIN = NDAC × MDAC × DOSR × DAC_fs
	// DAC_MOD_CLK = CODEC_CLKIN / ( NDAC × MDAC )
	// DAC_fs = 48000 = CODEC_CLKIN / ( NDAC × MDAC × DOSR )
	// DOSR must be multiple of 8and 2.8 MHz < DOSR × DAC_f S < 6.2 MHz
	// NDAC as large as possible while maintaining MDAC x DOSR / 32 >= RC (default PRB_P1 thus RC = 8)

	const unsigned NDAC = 2;
	const unsigned MDAC = 4;
	const unsigned DOSR = 64;

	// Set NDAC clock divider and power up.
	DAC3101_REGWRITE(DAC3101_NDAC_VAL, 0x80 | NDAC);
	// Set MDAC clock divider and power up.
	DAC3101_REGWRITE(DAC3101_MDAC_VAL, 0x80 | MDAC);
	// Set OSR clock divider.
	DAC3101_REGWRITE(DAC3101_DOSR_VAL_LSB, DOSR & 0xff);
	DAC3101_REGWRITE(DAC3101_DOSR_VAL_MSB, (DOSR & 0x0300) >> 8);
	// Set PLL_CLKIN and CODEC_CLKIN to MCLK pin
	DAC3101_REGWRITE(DAC3101_CLK_GEN_MUX, 0x00);

	// Set CODEC interface mode: I2S, 32 bit, slave mode (BCLK, WCLK both inputs).
	DAC3101_REGWRITE(DAC3101_CODEC_IF, 0x30);

	// Set register page to 1
	DAC3101_REGWRITE(DAC3101_PAGE_CTRL, 0x01);
	// Program common-mode voltage to mid scale 1.65V.
	DAC3101_REGWRITE(DAC3101_HP_DRVR, 0x14);
	// Program headphone-specific depop settings.
	// De-pop, Power on = 800 ms, Step time = 4 ms
	DAC3101_REGWRITE(DAC3101_HP_DEPOP, 0x4E);
	// Program routing of DAC output to the output amplifier (headphone/lineout or speaker)
	// LDAC routed to left channel mixer amp, RDAC routed to right channel mixer amp
	DAC3101_REGWRITE(DAC3101_DAC_OP_MIX, 0x44);
	// Unmute and set gain of output driver
	// Unmute HPL, set gain = 0 db
	DAC3101_REGWRITE(DAC3101_HPL_DRVR, 0x06);
	// Unmute HPR, set gain = 0 dB
	DAC3101_REGWRITE(DAC3101_HPR_DRVR, 0x06);
	// Unmute Left Class-D, set gain = 12 dB
	DAC3101_REGWRITE(DAC3101_SPKL_DRVR, 0x0C);
	// Unmute Right Class-D, set gain = 12 dB
	DAC3101_REGWRITE(DAC3101_SPKR_DRVR, 0x0C);
	// Power up output drivers
	// HPL and HPR powered up
	DAC3101_REGWRITE(DAC3101_HP_DRVR, 0xD4);
	// Power-up L and R Class-D drivers
	DAC3101_REGWRITE(DAC3101_SPK_AMP, 0xC6);
	// Enable HPL output analog volume, set = -9 dB
	DAC3101_REGWRITE(DAC3101_HPL_VOL_A, 0x92);
	// Enable HPR output analog volume, set = -9 dB
	DAC3101_REGWRITE(DAC3101_HPR_VOL_A, 0x92);
	// Enable Left Class-D output analog volume, set = -9 dB
	DAC3101_REGWRITE(DAC3101_SPKL_VOL_A, 0x92);
	// Enable Right Class-D output analog volume, set = -9 dB
	DAC3101_REGWRITE(DAC3101_SPKR_VOL_A, 0x92);

	delay_milliseconds(100);

	// Power up DAC
	// Set register page to 0
	DAC3101_REGWRITE(DAC3101_PAGE_CTRL, 0x00);
	// Power up DAC channels and set digital gain
	// Powerup DAC left and right channels (soft step enabled)
	DAC3101_REGWRITE(DAC3101_DAC_DAT_PATH, 0xD4);
	// DAC Left gain = 0dB
	DAC3101_REGWRITE(DAC3101_DACL_VOL_D, 0x00);
	// DAC Right gain = 0dB
	DAC3101_REGWRITE(DAC3101_DACR_VOL_D, 0x00);
	// Unmute digital volume control
	// Unmute DAC left and right channels
	DAC3101_REGWRITE(DAC3101_DAC_VOL, 0x00);

	delay_milliseconds(100);
}

static void pll_init(soc_peripheral_t i2c_dev)
{
	;
}

void audio_hw_config(soc_peripheral_t i2c_dev)
{
    pll_init(i2c_dev);
    initDAC3101(i2c_dev);
}
