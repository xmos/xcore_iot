How to use the SPI library as SPI slave
========================================

.. version:: 1.0.1

Summary
-------

This application note shows how to use the SPI library to make the
xCORE use an SPI bus as SPI slave. The application provides a register
file that can be read and written by the internal application and by
the SPI master using a simple command set. The code is
run in simulation with an SPI master output looped-back onto the SPI
slave input to show the bus functioning.

Required tools and libraries
............................

* xTIMEcomposer Tools - Version 14.0 
* XMOS SPI library - Version 3.0.0

Required hardware
.................

This application note is designed to run in simulation so requires no
XMOS hardware.

Prerequisites
.............

  - This document assumes familiarity with the XMOS xCORE
    architecture, the SPI bus protocol, the XMOS tool chain and the xC
    language. Documentation related to these aspects which are not
    specific  to this application note are linked to in the references appendix.

  - For descriptions of XMOS related terms found in this document
    please see the XMOS Glossary [#]_.

  - For the full API listing of the XMOS SPI Device Library please see
    the library user guide [#]_.

  .. [#] http://www.xmos.com/published/glossary

  .. [#] http://www.xmos.com/support/libraries/lib_spi


