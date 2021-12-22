.. include:: ../../../substitutions.rst

#################
|I2S| HIL Library
#################

A software defined library that allows you to control an |I2S| bus via XCore ports. |I2S| is a digital data streaming interfaces particularly appropriate for transmission of audio data. The components in the library are controlled via C and can either act as |I2S| master or |I2S| slave.

********
Core API
********

The following structures and functions are used by an |I2S| master or slave instance.

.. doxygengroup:: hil_i2s_core
   :content-only:

***************
Master Mode API
***************

The following structures and functions are used to initialize and start an |I2S| master instance.

.. doxygengroup:: hil_i2s_master
   :content-only:

**************
Slave Mode API
**************

The following structures and functions are used to initialize and start an |I2S| slave instance.

.. doxygengroup:: hil_i2s_slave
   :content-only:

