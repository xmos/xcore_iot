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

	delay_milliseconds(400);

	// Set register page to 0
	DAC3101_REGWRITE(DAC3101_PAGE_CTRL, 0x00);
	// Initiate SW reset (PLL is powered off as part of reset)
	DAC3101_REGWRITE(DAC3101_SW_RST, 0x01);
	delay_milliseconds(10);

	// We are supplying a 24.576MHz on mclk
	// PLL_CLK = 24.576MHz

	// CODEC_CLKIN = NDAC × MDAC × DOSR × DAC_fs
	// DAC_CLK = PLL_CLK / NDAC
	//		   = 24.576MHz / 2
	//		   = 12.288MHz
	// DAC_MOD_CLK = DAC_CLK / MDAC
	//			   = 12.288MHz / 4
	//			   = 3.072MHz
	// DAC_fs = DAC_MOD_CLK / DOSR
	//		  = 3.072MHz / 64
	//		  = 48KHz

	const unsigned NDAC = 2;
	const unsigned MDAC = 4;
	const unsigned DOSR = 64;

    // Set PLL_CLKIN = MCLK (device pin), CODEC_CLKIN = MCLK (device pin)
	DAC3101_REGWRITE(DAC3101_CLK_GEN_MUX, 0x00);
	// Set NDAC clock divider and power up.
	DAC3101_REGWRITE(DAC3101_NDAC_VAL, 0x80 | NDAC);
	// Set MDAC clock divider and power up.
	DAC3101_REGWRITE(DAC3101_MDAC_VAL, 0x80 | MDAC);
	// Set OSR clock divider.
	DAC3101_REGWRITE(DAC3101_DOSR_VAL_LSB, DOSR & 0xff);
	DAC3101_REGWRITE(DAC3101_DOSR_VAL_MSB, (DOSR & 0x0300) >> 8);

	// Set CODEC interface mode: I2S, 24 bit, slave mode (BCLK, WCLK both inputs).
//	DAC3101_REGWRITE(DAC3101_CODEC_IF, 0x20);

#if 0
	// Set register page to 1
	DAC3101_REGWRITE(DAC3101_PAGE_CTRL, 0x01);

    // Program headphone-specific depop settings.
    // De-pop, Power on = 800 ms, Step time = 4 ms
    DAC3101_REGWRITE(DAC3101_HP_DEPOP, 0x4E);
    // Program routing of DAC output to the output amplifier (headphone/lineout or speaker)
    // LDAC routed to left channel mixer amp, RDAC routed to right channel mixer amp
    DAC3101_REGWRITE(DAC3101_DAC_OP_MIX, 0x44);
//	DAC3101_REGWRITE(DAC3101_DAC_OP_MIX, 0x88);
	// Enable HPL output analog volume, set = max
	DAC3101_REGWRITE(DAC3101_HPL_VOL_A, 0x80);
	// Enable HPR output analog volume, set = max
	DAC3101_REGWRITE(DAC3101_HPR_VOL_A, 0x80);
	// Unmute and set gain of output driver
	// Unmute HPL, set gain = 12 db
	DAC3101_REGWRITE(DAC3101_HPL_DRVR, 0x0C);
	// Unmute HPR, set gain = 12 dB
	DAC3101_REGWRITE(DAC3101_HPR_DRVR, 0x0C);

	// Program common-mode voltage to mid scale 1.65V.
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
#else


	// Set register page to 1
	DAC3101_REGWRITE(DAC3101_PAGE_CTRL, 0x01);
	// Program common-mode voltage to mid scale 1.65V.
	DAC3101_REGWRITE(DAC3101_HP_DRVR, 0x14);
	// Program headphone-specific depop settings.
	// De-pop, Power on = 800 ms, Step time = 4 ms
	DAC3101_REGWRITE(DAC3101_HP_DEPOP, 0x4E);
	// Program routing of DAC output to the output amplifier (headphone/lineout or speaker)
	// LDAC routed to left channel mixer amp, RDAC routed to right channel mixer amp
	DAC3101_REGWRITE(DAC3101_DAC_OP_MIX, 0x88);
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
//	DAC3101_REGWRITE(DAC3101_HP_DRVR, 0xD4);
	DAC3101_REGWRITE(DAC3101_HP_DRVR, 0xC4);
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

#endif
	delay_milliseconds(100);

	// Power up DAC
	// Set register page to 0
	DAC3101_REGWRITE(DAC3101_PAGE_CTRL, 0x00);
	// Power up DAC channels and set digital gain
	// Powerup DAC left and right channels (soft step enabled)
	DAC3101_REGWRITE(DAC3101_DAC_DAT_PATH, 0xD4);
	// DAC Left gain = 0dB
	DAC3101_REGWRITE(DAC3101_DACL_VOL_D, 0x30);
	// DAC Right gain = 0dB
	DAC3101_REGWRITE(DAC3101_DACR_VOL_D, 0x30);
	// Unmute digital volume control
	// Unmute DAC left and right channels
	DAC3101_REGWRITE(DAC3101_DAC_VOL, 0x00);

	delay_milliseconds(100);
}



static void initDAC3101_test(soc_peripheral_t i2c_dev)
{
    unsigned char data = 0;
	i2c_regop_res_t i2c_res;

	delay_milliseconds(400);

	// We are supplying a 24.576MHz on mclk
	// PLL_CLK = 24.576MHz

	// CODEC_CLKIN = NDAC × MDAC × DOSR × DAC_fs
	// DAC_CLK = PLL_CLK / NDAC
	//		   = 24.576MHz / 2
	//		   = 12.288MHz
	// DAC_MOD_CLK = DAC_CLK / MDAC
	//			   = 12.288MHz / 4
	//			   = 3.072MHz
	// DAC_fs = DAC_MOD_CLK / DOSR
	//		  = 3.072MHz / 64
	//		  = 48KHz

	const unsigned NDAC = 2;
	const unsigned MDAC = 4;
	const unsigned DOSR = 64;

//	 static const uint8_t page_0[] = {
//	         0, /* start writing at register 0 */
//	         0  /* page 0 */
//	 };
	// Set register page to 0
	DAC3101_REGWRITE(DAC3101_PAGE_CTRL, 0x00);

//	 static const uint8_t reset[] = {
//	         1, /* start writing at register 1 */
//	         1  /* reset the DAC */
//	 };
	// Initiate SW reset (PLL is powered off as part of reset)
	DAC3101_REGWRITE(DAC3101_SW_RST, 0x01);
	delay_milliseconds(10);

//	 static const uint8_t divider_config[] = {
//	         11,          /* start writing at register 11 */
//	         0x80 | 2,    /* Register 11 - enable NDAC, NDAC = 2 */
//	         0x80 | 4,    /* Register 12 - enable MDAC, MDAC = 4 */
//	         0,           /* Register 13 - DOSR[9:8] = 0 */
//	         64           /* Register 13 - DOSR[7:0] = 64 */
//	 };
	// Set NDAC clock divider and power up.
	DAC3101_REGWRITE(DAC3101_NDAC_VAL, 0x80 | NDAC);
	// Set MDAC clock divider and power up.
	DAC3101_REGWRITE(DAC3101_MDAC_VAL, 0x80 | MDAC);
	// Set OSR clock divider.
	DAC3101_REGWRITE(DAC3101_DOSR_VAL_LSB, DOSR & 0xff);
	DAC3101_REGWRITE(DAC3101_DOSR_VAL_MSB, (DOSR & 0x0300) >> 8);

//	 static const uint8_t page_1[] = {
//	         0, /* start writing at register 0 */
//	         1  /* page 0 */
//	 };
	// Set register page to 1
	DAC3101_REGWRITE(DAC3101_PAGE_CTRL, 0x01);

//	 static const uint8_t hp_driver_config1[] = {
//	         35,    /* start writing at register 35 */
//	         0x88,  /* route DAC outputs directly to headphone driver */
//	         0x80,  /* set headphone left to max volume */
//	         0x80,  /* set headphone right to max volume */
//	         0x7F,  /* skip, write reset value */
//	         0x7F,  /* skip, write reset value */
//	         0x06,  /* unmute left headphone driver, gain 0 dB */
//	         0x06,  /* unmute right headphone driver, gain 0 dB */
//	 };
	// Program routing of DAC output to the output amplifier (headphone/lineout or speaker)
	// LDAC routed to left channel mixer amp, RDAC routed to right channel mixer amp
	DAC3101_REGWRITE(DAC3101_DAC_OP_MIX, 0x88);
	// Enable HPL output analog volume, set = max
	DAC3101_REGWRITE(DAC3101_HPL_VOL_A, 0x80);
	// Enable HPR output analog volume, set = max
	DAC3101_REGWRITE(DAC3101_HPR_VOL_A, 0x80);
	// Unmute and set gain of output driver
	// Unmute HPL, set gain = 12 db
	DAC3101_REGWRITE(DAC3101_HPL_DRVR, 0x06);
	// Unmute HPR, set gain = 12 dB
	DAC3101_REGWRITE(DAC3101_HPR_DRVR, 0x06);

//	 static const uint8_t hp_driver_config2[] = {
//	         31,   /* start writing at register 31 */
//	         0xC4, /* power up L/R headphone drivers, common mode voltage to 1.35V */
//	 };
	// Program common-mode voltage to mid scale 1.65V.
	DAC3101_REGWRITE(DAC3101_HP_DRVR, 0xC4);

	delay_milliseconds(10);
	//	 static const uint8_t page_0[] = {
	//	         0, /* start writing at register 0 */
	//	         0  /* page 0 */
	//	 };

		// Set register page to 0
		DAC3101_REGWRITE(DAC3101_PAGE_CTRL, 0x00);

//		 static const uint8_t dac_config[] = {
//		         63,          /* start writing at register 63 */
//		         0xD4,        /* power up and route both DAC channels */
//		         0x00,        /* unmute both DAC channels, independent volume control */
//		         -60,           /* left volume is -30 dB */
//		         -60,           /* right volume is -30 dB */
//		 };
		// Power up DAC channels and set digital gain
		// Powerup DAC left and right channels (soft step enabled)
		DAC3101_REGWRITE(DAC3101_DAC_DAT_PATH, 0xD4);
		// Unmute digital volume control
		// Unmute DAC left and right channels
		DAC3101_REGWRITE(DAC3101_DAC_VOL, 0x00);
		// DAC Left gain = 0dB
		DAC3101_REGWRITE(DAC3101_DACL_VOL_D, -60);
		// DAC Right gain = 0dB
		DAC3101_REGWRITE(DAC3101_DACR_VOL_D, -60);
		delay_milliseconds(10);
}




static void pll_init(soc_peripheral_t i2c_dev)
{
	;	// PLL configuration is in bitstream.xc for tile 1
}

void audio_hw_config(soc_peripheral_t i2c_dev)
{
    pll_init(i2c_dev);
//    initDAC3101(i2c_dev);
    initDAC3101_test(i2c_dev);
}
