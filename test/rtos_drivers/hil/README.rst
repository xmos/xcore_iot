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

============  ================
Pin Desc      Connection
============  ================
GPIO I/O      X1D12 : X1D39
I2C SCL       SCL IOL : X1D36
I2C SDA       SDA IOL : X1D38
I2S DACD      DAC_DAT : X0D12
I2S ADCD      ADC_DAT : X0D13
I2S BCLK      BCLK : X0D22
I2S LRCLK     LRCLK : X0D23
============  ================

Wiring Diagram
==============

.. image:: images/wiring_diagram.png
    :align: left