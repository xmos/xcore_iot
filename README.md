# XCORE:registered: IOT Repository

[![Version](https://img.shields.io/github/v/release/xmos/xcore_iot?include_prereleases)](https://github.com/xmos/xcore_iot/releases/latest)
[![Issues](https://img.shields.io/github/issues/xmos/xcore_iot)](https://github.com/xmos/xcore_iot/issues)
[![Contributors](https://img.shields.io/github/contributors/xmos/xcore_iot)](https://github.com/xmos/xcore_iot/graphs/contributors)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square)](https://github.com/xmos/xcore_iot/pulls)

XCORE-IOT is a collection of C/C++ software libraries designed to simplify and accelerate application development on xcore processors. It is composed of the following components:

- Peripheral IO libraries including; UART, I2C, I2S, SPI, QSPI, PDM microphones, and USB. These libraries support bare-metal and RTOS application development.
- Libraries core to DSP applications, including vectorized math.  These libraries support bare-metal and RTOS application development. 
- Voice processing libraries including; adaptive echo cancellation, adaptive gain control, noise suppression, interference cancellation (IC), and voice activity detection. These libraries support bare-metal and RTOS application development.
- Libraries that enable [multi-core FreeRTOS development](https://www.freertos.org/symmetric-multiprocessing-introduction.html) on xcore including a wide array of RTOS drivers and middleware.
- Code Examples - Examples showing a variety of xcore features based on bare-metal and FreeRTOS programming.

XCORE-IOT is designed to be used in conjunction with the xcore.ai Explorer board evaluation kit. The example applications compile targeting this board. Further information about the Explorer board and xcore.ai devices is available to on [www.xmos.ai](https://www.xmos.ai/).

## Build Status

Build Type       |    Status     |
-----------      | --------------|
CI (Linux)       | ![CI](https://github.com/xmos/xcore_iot/actions/workflows/ci_examples.yml/badge.svg?branch=develop&event=push) |
CI (Linux)       | ![CI](https://github.com/xmos/xcore_iot/actions/workflows/ci_tests.yml/badge.svg?branch=develop&event=push) |
Docs             | ![CI](https://github.com/xmos/xcore_iot/actions/workflows/docs.yml/badge.svg?branch=develop&event=push) |

## Cloning

Some dependent components are included as git submodules. These can be obtained by cloning this repository with the following command:

    git clone --recurse-submodules git@github.com:xmos/xcore_iot.git

## Documentation

See the [official documentation](https://www.xmos.ai/documentation/XM-014660-PC-2/html/) for more information including:

- Instructions for modifying the software
- Programming tutorials
- API references

## License

This Software is subject to the terms of the [XMOS Public Licence: Version 1](https://github.com/xmos/xcore_iot/blob/develop/LICENSE.rst). Copyrights and licenses for third party components can be found in [Copyrights and Licenses](https://github.com/xmos/xcore_iot/blob/develop/doc/shared/legal.rst).

