.. include:: ../../../../substitutions.rst

################
QSPI HIL Library
################

A software defined QSPI (quad serial peripheral interface) library that allows you to read and write to a QSPI peripheral via the XCore ports.

All QSPI functions can be accessed via the ``qspi_flash.h`` or ``qspi_io.h`` header:

.. code-block:: c
   
   #include <qspi_flash.h>
   #include <qspi_io.h>

.. toctree::
   :maxdepth: 2
   :includehidden:

   qspi_flash.rst
   qspi_io.rst
