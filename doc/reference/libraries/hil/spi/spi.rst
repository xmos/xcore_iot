.. include:: ../../../../substitutions.rst

###############
SPI HIL Library
###############

A software defined SPI (serial peripheral interface) library that allows you to control a SPI bus via the XCore GPIO hardware-response ports. SPI is a four-wire hardware bi-directional serial interface. The components in the library are controlled via C and can either act as SPI master or slave.

The SPI bus can be used by multiple tasks within the XCore device and (each addressing the same or different slaves) and is compatible with other slave devices on the same bus.

The SPI protocol requires a clock, one or more slave selects and either one or two data wires.

.. _spi_wire_table:

.. list-table:: SPI data wires
     :class: vertical-borders horizontal-borders

     * - *SCLK*
       - Clock line, driven by the master
     * - *MOSI*
       - Master Output, Slave Input data line, driven by the master
     * - *MISO*
       - Master Input, Slave Output data line, driven by the slave
     * - *SS*
       - Slave select line, driven by the master

All SPI functions can be accessed via the ``spi.h`` header:

.. code-block:: c
   
   #include <spi.h>

.. toctree::
   :maxdepth: 2
   :includehidden:

   spi_master.rst
   spi_slave.rst
