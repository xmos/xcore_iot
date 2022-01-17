.. include:: ../../../../substitutions.rst

#################
|I2S| HIL Library
#################

A software defined library that allows you to control an |I2S| (Inter-IC Sound) bus via XCore ports. |I2S| is a digital data streaming interfaces particularly appropriate for transmission of audio data. The components in the library are controlled via C and can either act as |I2S| master or |I2S| slave.

And archived version of the original |I2S| specification can be found here:

https://web.archive.org/web/20070102004400/http://www.nxp.com/acrobat_download/various/I2SBUS.pdf

.. note::

   The TDM protocol is not yet supported by this library.

|I2S| is a protocol between two devices where one is the *master* and one is the *slave* . The protocol is made up of four signals shown
in :ref:`i2s_wire_table`.

.. _i2s_wire_table:

.. list-table:: |I2S| data wires
     :class: vertical-borders horizontal-borders

     * - *MCLK*
       - Clock line, driven by external oscillator
     * - *BCLK*
       - Bit clock. This is a fixed divide of the *MCLK* and is driven
         by the master.
     * - *LRCLK* (or *WCLK*)
       - Word clock (or word select). This is driven by the master.
     * - *DATA*
       - Data line, driven by one of the slave or master depending on
         the data direction. There may be several data lines in
         differing directions.

All |I2S| functions can be accessed via the ``i2s.h`` header:

.. code-block:: c
   
   #include <i2s.h>

.. toctree::
   :maxdepth: 2
   :includehidden:

   i2s_common.rst
   i2s_master.rst
   i2s_slave.rst
