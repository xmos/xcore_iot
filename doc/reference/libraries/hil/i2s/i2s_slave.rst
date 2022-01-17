.. include:: ../../../../substitutions.rst

***********
|I2S| Slave
***********

|I2S| Slave Usage
=================

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

|I2S| Slave API
===============

The following structures and functions are used to initialize and start an |I2S| slave instance.

.. doxygengroup:: hil_i2s_slave
   :content-only:

