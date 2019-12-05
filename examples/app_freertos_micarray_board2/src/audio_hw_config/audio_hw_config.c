// Copyright (c) 2019, XMOS Ltd, All rights reserved

/* BSP/bitstream headers */
#include "i2c_driver.h"

/* Library headers */
#include "soc.h"

/* Application headers */
#include "CS43L21.h"

#define I2S_CHANS_DAC 2


static void initCS43L21(soc_peripheral_t i2c_dev)
{
    i2c_regop_res_t res;

    for (int i = CS43L21_I2C_ADDR; i < CS43L21_I2C_ADDR + (I2S_CHANS_DAC / 2); i++) {
        /* Power control (turn DAC off)
         * |7:reserved|6:PDN_DACB|5:PDN_DACA|4..1:reserved|0:PDN|
         */
        res = i2c_driver_write_reg(i2c_dev, i, CS43L21_REG_POWER_CTL, 0x01);
        xassert(res == I2C_REGOP_SUCCESS);
        /* I2S Mode
         * |7:reserved|6:M/S|5..3:fmt|2..0:reserved|
         */
        // I2S mode, up to 24-bit data
        res = i2c_driver_write_reg(i2c_dev, i, CS43L21_REG_IFACE_CTL, 0x08);
        xassert(res == I2C_REGOP_SUCCESS);
        /* Speed Control
         * |7:AUTO|6..5:SPEED|4:tristate|3..1:reserved|0:MCLKDIV2|
         */
        res = i2c_driver_write_reg(i2c_dev, i, CS43L21_REG_SPEED_CTL, 0b10000001);
        xassert(res == I2C_REGOP_SUCCESS);
        /* DAC Output Control
         * |7..5:ampgain|4:DAC_SNGVOL|3:INV_PCMB|2:INV_PCMA|1:DACB_MUTE|0:DACA_MUTE|
         */
        res = i2c_driver_write_reg(i2c_dev, i, CS43L21_REG_DAC_OUT_CTL, 0b00000000);
        xassert(res == I2C_REGOP_SUCCESS);
        /* DAC Control (disable DSP)
         * |7..6:DATA_SEL|5:FREEZE|4:reserved|3:DEEMPH|2:AMUTE|1::0:DAC_SZC|
         */
        res = i2c_driver_write_reg(i2c_dev, i, CS43L21_REG_DAC_CTL, 0x00);
        xassert(res == I2C_REGOP_SUCCESS);
        /* Power control (turn DAC on)
         * |7:reserved|6:PDN_DACB|5:PDN_DACA|4..1:reserved|0:PDN|
         */
        res = i2c_driver_write_reg(i2c_dev, i, CS43L21_REG_POWER_CTL, 0x00);
        xassert(res == I2C_REGOP_SUCCESS);
    }
}

static void pll_init(soc_peripheral_t i2c_dev)
{
    // SI5351A Register Addresses
    #define SI5351A_OE_CTRL      (0x03) // Register 3  - Output Enable Control
    #define SI5351A_FANOUT_EN    (0xBB) // Register 187 - Fanout Enable Control

    #define SI5351A_MS0_R0_DIV   (0x2C) /* Register 44 - Multisynth0 Parameters:
                                         *  - R0_DIV[2:0]
                                         *  - MS0_DIVBY4[1:0]
                                         *  - MS0_P1[17:16]
                                         */
    #define SI5351A_MS2_R2_DIV   (0x3C) /* Register 60 - Multisynth2 Parameters:
                                         *  - R2_DIV[2:0]
                                         *  - MS2_DIVBY4[1:0]
                                         *  - MS2_P1[17:16]
                                         */

    #define SI5351A_CLK0_CTRL    (0x10) // Register 16 - CLK0 Control
    #define SI5351A_MS0_P1_UPPER (0x2D) /* Register 45 - Multisynth0 Parameters:
                                         *  - MS0_P1[15:8]
                                         */
    #define SI5351A_MS0_P2_LOWER (0x31) /* Register 49 - Multisynth0 Parameters:
                                         *  - MS0_P2[7:0]
                                         */

    #define SI5351A_CLK2_CTRL    (0x12) // Register 18 - CLK2 Control
    #define SI5351A_MS2_P1_UPPER (0x3D) /* Register 61 - Multisynth2 Parameters:
                                         *  - MS2_P1[15:8]
                                         */
    #define SI5351A_MS2_P2_LOWER (0x41) /* Register 65 - Multisynth2 Parameters:
                                         *  - MS2_P2[7:0]
                                         */
    i2c_regop_res_t res;
    // Configure SI5351A clock generator
      int clock_gen_i2c_address = 0x62;
    // Disable the CLK0 output (to xCORE MCLK in).
    res = i2c_driver_write_reg(i2c_dev, clock_gen_i2c_address, SI5351A_OE_CTRL, 0xFD);

    // Enable Fanout of MS0 to other outputs.
    res = i2c_driver_write_reg(i2c_dev, clock_gen_i2c_address, SI5351A_FANOUT_EN, 0xD0);

    /* Change R0 divider to divide by 2 instead of divide by 1.
     * This stays at this value.
     */
    res = i2c_driver_write_reg(i2c_dev, clock_gen_i2c_address, SI5351A_MS0_R0_DIV, 0x10);
    res = i2c_driver_write_reg(i2c_dev, clock_gen_i2c_address, SI5351A_MS2_R2_DIV, 0x30);

    /* MCLK = 24.576MHz (12,24,48,96,192kHz)
     * Sets powered up, integer mode, src PLLA, not inverted,
     * Sel MS0 as src for CLK0 o/p, 4mA drive strength
     */
    res = i2c_driver_write_reg(i2c_dev, clock_gen_i2c_address, SI5351A_CLK0_CTRL, 0x4D);
    res = i2c_driver_write_reg(i2c_dev, clock_gen_i2c_address, SI5351A_CLK2_CTRL, 0x69);
    // Sets relevant bits of P1 divider setting
    res = i2c_driver_write_reg(i2c_dev, clock_gen_i2c_address, SI5351A_MS0_P1_UPPER, 0x05);

    /* Now we write the lower bits of Multisynth Parameter P2.
     * This updates all the divider values into the Multisynth block.
     * The other multisynth parameters are correct so no need to write them.
     */
    res = i2c_driver_write_reg(i2c_dev, clock_gen_i2c_address, SI5351A_MS0_P2_LOWER, 0x00);

    // Wait a bit for Multisynth output to settle.
    delay_microseconds(1000);

    /* Enable all the clock outputs now we've finished changing the settings.
     * This will output 24.576MHz on CLK0 to xcore
     */
    res = i2c_driver_write_reg(i2c_dev, clock_gen_i2c_address, SI5351A_OE_CTRL, 0xF8);
}

void audio_hw_config(soc_peripheral_t i2c_dev)
{
    pll_init(i2c_dev);
    initCS43L21(i2c_dev);
}
