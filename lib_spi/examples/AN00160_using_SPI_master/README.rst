How to use the SPI library as SPI master
========================================

.. version:: 1.0.1

Summary
-------

This application note shows how to use the SPI library to make the
xCORE drive an SPI bus as SPI master. The application is the simplest
example of setting up the library and performing a couple of
transactions. The code can then be run in simulation to see the
outputted waveforms.

The note covers both the synchronous and asynchronous use of the SPI
master components provided from the library.

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


