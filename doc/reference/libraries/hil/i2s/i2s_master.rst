.. include:: ../../../../substitutions.rst


************
|I2S| Master
************

|I2S| Master Usage
==================

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

|I2S| Master API
================

The following structures and functions are used to initialize and start an |I2S| master instance.

.. doxygengroup:: hil_i2s_master
   :content-only:
