.. include:: ../../../substitutions.rst

###############
SPI HIL Library
###############

A software defined SPI (serial peripheral interface) library that allows you to control a SPI bus via the XCore GPIO hardware-response ports. SPI is a four-wire hardware bi-directional serial interface. The components in the library are controlled via C and can either act as SPI master or slave.

The SPI bus can be used by multiple tasks within the XCore device and (each addressing the same or different slaves) and is compatible with other slave devices on the same bus.

***************
Master Mode API
***************

The following structures and functions are used to initialize and start an SPI master instance.

.. doxygengroup:: hil_spi_master
   :content-only:

**************
Slave Mode API
**************

The following structures and functions are used to initialize and start an SPI slave instance.

.. doxygengroup:: hil_spi_slave
   :content-only:

