##############
Device Control
##############


The Device Control Service provides the ability to configure and control an XMOS device from a host over a number of transport layers.
Features of the service include:

- Simple read/write API
- Fully acknowledged protocol
- Includes different transports including I2C and USB.
  
The table below shows combinations of host and transport mechanisms that are currently supported. 
Adding new transport layers and/or hosts is straightforward where the hardware supports it.

.. list-table:: Supported Device Control Library Transports
 :header-rows: 1

 * - Host
   - I2C
   - USB
 * - PC / Windows
   - 
   - Yes
 * - PC / OSX
   -
   - Yes
 * - Raspberry Pi / Linux
   - Yes
   - Yes
 * - xCORE
   - Yes
   - 

.. toctree::
   :maxdepth: 1

   device_control_shared.rst
   device_control_xcore.rst
   device_control_host_api.rst
   device_control_protocol.rst
