#################
RTOS Driver Tests
#################

The RTOS driver tests are designed to regression test RTOS driver behavior for the following drivers:

- gpio
- i2c
- i2s
- intertile
- mic_array
- qspi_flash
- swmem

These tests assume that the associated RTOS and HILs used have been verified by their own localized separate testing.

These tests should be run whenever the code or submodules in ``modules\rtos`` or ``modules\hil`` are changed.

**************
Hardware Setup
**************

The target hardware for these tests is the XCORE-AI-EXPLORER board.

To setup the board for testing, the following connections must be made:

============  ================  ================  =====================================
Pin Desc hil  Pin Desc hil_add  Connection        Port Name
============  ================  ================  =====================================
GPIO I/O      Unused            X1D02 : X1D39     T1 XS1_PORT_4A Pin 0 : T1 XS1_PORT_1P
I2C SCL       UART TX/RX        SCL IOL : X1D36   T0 XS1_PORT_1N : T1 XS1_PORT_1M
I2C SDA       SPI CS            SDA IOL : X1D38   T0 XS1_PORT_1O : T1 XS1_PORT_1O
I2S DACD      SPI MOSI          DAC_DAT : X0D12   T1 XS1_PORT_1A : T0 XS1_PORT_1E
I2S ADCD      SPI MISO          ADC_DAT : X0D13   T1 XS1_PORT_1N : T0 XS1_PORT_1F
I2S BCLK      Unused            BCLK : X0D22      T1 XS1_PORT_1C : T0 XS1_PORT_1G
I2S LRCLK     SPI SCLK          LRCLK : X0D23     T1 XS1_PORT_1B : T0 XS1_PORT_1H
============  ================  ================  =====================================

Wiring Diagram
==============

.. image:: images/wiring_diagram.png
    :align: left
