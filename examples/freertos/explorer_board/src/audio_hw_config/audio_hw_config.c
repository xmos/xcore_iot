// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved

/* BSP/bitstream headers */
#include "DAC3204.h"
#include "i2c_driver.h"

/* Library headers */
#include "soc.h"

/* Application headers */

#define I2S_CHANS_DAC 2


static void initDAC3204(soc_peripheral_t i2c_dev)
{
	// Set register page to 0
	AIC3204_REGWRITE(AIC3204_PAGE_CTRL, 0x00);

	// Initiate SW reset (PLL is powered off as part of reset)
	AIC3204_REGWRITE(AIC3204_SW_RST, 0x01);

	// Program clock settings

	// Default is CODEC_CLKIN is from MCLK pin. Don't need to change this.
	// Power up NDAC and set to 1
	AIC3204_REGWRITE(AIC3204_NDAC, 0x81);
	// Power up MDAC and set to 4
	AIC3204_REGWRITE(AIC3204_MDAC, 0x84);
	// Power up NADC and set to 1
	AIC3204_REGWRITE(AIC3204_NADC, 0x81);
	// Power up MADC and set to 4
	AIC3204_REGWRITE(AIC3204_MADC, 0x84);
	// Program DOSR = 128
	AIC3204_REGWRITE(AIC3204_DOSR, 0x80);
	// Program AOSR = 128
	AIC3204_REGWRITE(AIC3204_AOSR, 0x80);
	// Set Audio Interface Config: I2S, 24 bits, slave mode, DOUT always driving.
	AIC3204_REGWRITE(AIC3204_CODEC_IF, 0x20);
	// Program the DAC processing block to be used - PRB_P1
	AIC3204_REGWRITE(AIC3204_DAC_SIG_PROC, 0x01);
	// Program the ADC processing block to be used - PRB_R1
	AIC3204_REGWRITE(AIC3204_ADC_SIG_PROC, 0x01);
	// Select Page 1
	AIC3204_REGWRITE(AIC3204_PAGE_CTRL, 0x01);
	// Enable the internal AVDD_LDO:
	AIC3204_REGWRITE(AIC3204_LDO_CTRL, 0x09);
	//
	// Program Analog Blocks
	// ---------------------
	//
	// Disable Internal Crude AVdd in presence of external AVdd supply or before powering up internal AVdd LDO
	AIC3204_REGWRITE(AIC3204_PWR_CFG, 0x08);
	// Enable Master Analog Power Control
	AIC3204_REGWRITE(AIC3204_LDO_CTRL, 0x01);
	// Set Common Mode voltages: Full Chip CM to 0.9V and Output Common Mode for Headphone to 1.65V and HP powered from LDOin @ 3.3V.
	AIC3204_REGWRITE(AIC3204_CM_CTRL, 0x33);
	// Set PowerTune Modes
	// Set the Left & Right DAC PowerTune mode to PTM_P3/4. Use Class-AB driver.
	AIC3204_REGWRITE(AIC3204_PLAY_CFG1, 0x00);
	AIC3204_REGWRITE(AIC3204_PLAY_CFG2, 0x00);
	// Set ADC PowerTune mode PTM_R4.
	AIC3204_REGWRITE(AIC3204_ADC_PTM, 0x00);
	// Set MicPGA startup delay to 3.1ms
	AIC3204_REGWRITE(AIC3204_AN_IN_CHRG, 0x31);
	// Set the REF charging time to 40ms
	AIC3204_REGWRITE(AIC3204_REF_STARTUP, 0x01);
	// HP soft stepping settings for optimal pop performance at power up
	// Rpop used is 6k with N = 6 and soft step = 20usec. This should work with 47uF coupling
	// capacitor. Can try N=5,6 or 7 time constants as well. Trade-off delay vs “pop” sound.
	AIC3204_REGWRITE(AIC3204_HP_START, 0x25);
	// Route Left DAC to HPL
	AIC3204_REGWRITE(AIC3204_HPL_ROUTE, 0x08);
	// Route Right DAC to HPR
	AIC3204_REGWRITE(AIC3204_HPR_ROUTE, 0x08);
	// We are using Line input with low gain for PGA so can use 40k input R but lets stick to 20k for now.
	// Route IN2_L to LEFT_P with 20K input impedance
	AIC3204_REGWRITE(AIC3204_LPGA_P_ROUTE, 0x20);
	// Route IN2_R to LEFT_M with 20K input impedance
	AIC3204_REGWRITE(AIC3204_LPGA_N_ROUTE, 0x20);
	// Route IN1_R to RIGHT_P with 20K input impedance
	AIC3204_REGWRITE(AIC3204_RPGA_P_ROUTE, 0x80);
	// Route IN1_L to RIGHT_M with 20K input impedance
	AIC3204_REGWRITE(AIC3204_RPGA_N_ROUTE, 0x20);
	// Unmute HPL and set gain to 0dB
	AIC3204_REGWRITE(AIC3204_HPL_GAIN, 0x00);
	// Unmute HPR and set gain to 0dB
	AIC3204_REGWRITE(AIC3204_HPR_GAIN, 0x00);
	// Unmute Left MICPGA, Set Gain to 0dB.
	AIC3204_REGWRITE(AIC3204_LPGA_VOL, 0x00);
	// Unmute Right MICPGA, Set Gain to 0dB.
	AIC3204_REGWRITE(AIC3204_RPGA_VOL, 0x00);
	// Power up HPL and HPR drivers
	AIC3204_REGWRITE(AIC3204_OP_PWR_CTRL, 0x30);

	// Wait for 2.5 sec for soft stepping to take effect
	delay_milliseconds(2500);

	//
	// Power Up DAC/ADC
	// ----------------
	//
	// Select Page 0
	AIC3204_REGWRITE(AIC3204_PAGE_CTRL, 0x00);
	// Power up the Left and Right DAC Channels. Route Left data to Left DAC and Right data to Right DAC.
	// DAC Vol control soft step 1 step per DAC word clock.
	AIC3204_REGWRITE(AIC3204_DAC_CH_SET1, 0xd4);
	// Power up Left and Right ADC Channels, ADC vol ctrl soft step 1 step per ADC word clock.
	AIC3204_REGWRITE(AIC3204_ADC_CH_SET, 0xc0);
	// Unmute Left and Right DAC digital volume control
	AIC3204_REGWRITE(AIC3204_DAC_CH_SET2, 0x00);
	// Unmute Left and Right ADC Digital Volume Control.
	AIC3204_REGWRITE(AIC3204_ADC_FGA_MUTE, 0x00);

	delay_milliseconds(1);
}


static void pll_init(soc_peripheral_t i2c_dev)
{
	;	// PLL configuration is in bitstream.xc for tile 1
}

void audio_hw_config(soc_peripheral_t i2c_dev)
{
    pll_init(i2c_dev);
    initDAC3204(i2c_dev);
}
