.. include:: ../../../substitutions.rst

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

********
Core API
********

The following structures and functions are used by an |I2S| master or slave instance.

.. doxygengroup:: hil_i2s_core
   :content-only:

***********
Master Mode
***********

Master Usage
============

The following code snippet demonstrates the basic usage of an |I2S| master device.

.. code-block:: c

   #include <xs1.h>
   #include <i2s.h>

   port_t p_i2s_dout[1];
   port_t p_bclk;
   port_t p_lrclk;
   port_t p_mclk;
   xclock_t bclk;
   i2s_callback_group_t i2s_cb_group;

   // Setup ports and clocks
   p_i2s_dout[0] = PORT_I2S_DAC_DATA;
   p_bclk = PORT_I2S_BCLK;
   p_lrclk = PORT_I2S_LRCLK;
   p_mclk = PORT_MCLK_IN;
   bclk = I2S_CLKBLK;

   port_enable(p_mclk);
   port_enable(p_bclk);
   // NOTE:  p_lrclk does not need to be enabled by the caller

   // Setup callbacks
   //  NOTE: See API or SDK examples for more on using the callbacks
   i2s_cb_group.init = (i2s_init_t) i2s_init;
   i2s_cb_group.restart_check = (i2s_restart_check_t) i2s_restart_check;
   i2s_cb_group.receive = (i2s_receive_t) i2s_receive;
   i2s_cb_group.send = (i2s_send_t) i2s_send;
   i2s_cb_group.app_data = NULL;

   // Start the master device in this thread
   //  NOTE: You may wish to launch the slave device in a different thread.  
   //        See the XTC Tools documentation reference for lib_xcore.
   i2s_master(&i2s_cb_group, p_i2s_dout, 1, NULL, 0, p_bclk, p_lrclk, p_mclk, bclk);

Master API
==========

The following structures and functions are used to initialize and start an |I2S| master instance.

.. doxygengroup:: hil_i2s_master
   :content-only:

**********
Slave Mode
**********

Slave Usage
===========

The following code snippet demonstrates the basic usage of an |I2S| slave device.

.. code-block:: c

   #include <xs1.h>
   #include <i2s.h>

   // Setup ports and clocks
   port_t p_bclk  = XS1_PORT_1B;
   port_t p_lrclk = XS1_PORT_1C;
   port_t p_din [4] = {XS1_PORT_1D, XS1_PORT_1E, XS1_PORT_1F, XS1_PORT_1G};
   port_t p_dout[4] = {XS1_PORT_1H, XS1_PORT_1I, XS1_PORT_1J, XS1_PORT_1K};
   xclock_t bclk = XS1_CLKBLK_1;

   port_enable(p_bclk); 
   // NOTE:  p_lrclk does not need to be enabled by the caller
   
   // Setup callbacks
   //  NOTE: See API or SDK examples for more on using the callbacks
   i2s_callback_group_t i_i2s = {
            .init = (i2s_init_t) i2s_init,
            .restart_check = (i2s_restart_check_t) i2s_restart_check,
            .receive = (i2s_receive_t) i2s_receive,
            .send = (i2s_send_t) i2s_send,
            .app_data = NULL,
   };

   // Start the slave device in this thread
   //  NOTE: You may wish to launch the slave device in a different thread.  
   //        See the XTC Tools documentation reference for lib_xcore.
   i2s_slave(&i_i2s, p_dout, 4, p_din, 4, p_bclk, p_lrclk, bclk);

Slave API
=========

The following structures and functions are used to initialize and start an |I2S| slave instance.

.. doxygengroup:: hil_i2s_slave
   :content-only:

