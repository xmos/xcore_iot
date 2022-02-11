########################
FreeRTOS Getting Started
########################

This is the simplest buildable multitile FreeRTOS project for XCore. We encourage you to use this as a template for new projects. To start your own project copy the contents of this folder to your computer and begin developing.

*********************
Building the firmware
*********************

Run the following commands in the xcore_sdk root folder to build the getting_started firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make getting_started

.. tab:: Windows XTC Tools CMD prompt

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build
        $ cd build
        $ nmake getting_started

*********************
Running the firmware
*********************

From the xcore_sdk build folder run:

.. tab:: Linux and MacOS

    .. code-block:: console

        $ make run_getting_started

.. tab:: Windows

    .. code-block:: console

        $ nmake run_getting_started

*********************
Debugging the firmware with xgdb
*********************

From the xcore_sdk build folder run:

.. tab:: Linux and MacOS

    .. code-block:: console

        $ make debug_getting_started

.. tab:: Windows

    .. code-block:: console

        $ nmake debug_getting_started
