XCORE SDK change log
====================

0.11.0
------

  * Removed of the FreeRTOS cifar10 example application
  * Removed visual_wake_word bare-metal example application
  * Removed inferencing modules

0.10.0
------

Many enhancements and changes from the prior release.  The list below summarizes many of the changes:

  * Addition of UART library and RTOS driver
  * Addition of clock control library and RTOS driver
  * Addition of L2 cache library and RTOS driver
  * Redesign of mic array library to leverage VPU
  * Removal of the cifar10, hello_world, hotdog_not_hotdog and microspeech bare-metal example applications
  * Addition of explorer_board bare-metal example application
  * Removal of the person_detection FreeRTOS example application
  * iot_aws FreeRTOS example application redesigned and renamed
  * Addition of device_control, getting_started, ls_cache and dispatcher FreeRTOS example applicatiosn
  * Added USB support for FreeRTOS applications
  * Simplified SDK installation steps
  * Redesign of CMake build system
  * Documentation updates

0.9.4
-----

  * Reduced RAM used for models using multiple cores
  * Reduced RAM used for models containing Conv2D Depthwise operator

0.9.3
-----

  * Initial pre-Alpha release
