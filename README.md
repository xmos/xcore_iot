# XCORE:registered: SDK Repository

[![Version](https://img.shields.io/github/v/release/xmos/xcore_sdk?include_prereleases)](https://github.com/xmos/xcore_sdk/releases/latest)
[![Issues](https://img.shields.io/github/issues/xmos/xcore_sdk)](https://github.com/xmos/xcore_sdk/issues)
[![Contributors](https://img.shields.io/github/contributors/xmos/xcore_sdk)](https://github.com/xmos/xcore_sdk/graphs/contributors)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square)](https://github.com/xmos/xcore_sdk/pulls)

The XCORE SDK is a collection of C/C++ software libraries designed to simplify and accelerate application development on xcore processors. It is composed of the following components:

- Peripheral IO libraries including; UART, I2C, I2S, SPI, QSPI, PDM microphones, and USB. These libraries support bare-metal and RTOS application development.
- Libraries core to DSP applications, including vectorized math.  These libraries support bare-metal and RTOS application development. 
- Voice processing libraries including; adaptive echo cancellation, adaptive gain control, noise suppression, interference cancellation (IC), and voice activity detection. These libraries support bare-metal and RTOS application development.
- Libraries that enable [multi-core FreeRTOS development](https://www.freertos.org/symmetric-multiprocessing-introduction.html) on xcore including a wide array of RTOS drivers and middleware.
- Code Examples - Examples showing a variety of xcore features based on bare-metal and FreeRTOS programming.

The SDK is designed to be used in conjunction with the xcore.ai Explorer board evaluation kit. The example applications compile targeting this board. Further information about the Explorer board and xcore.ai devices is available to on [www.xmos.ai](https://www.xmos.ai/).

## Build Status

Build Type       |    Status     |
-----------      | --------------|
CI (Linux)       | ![CI](https://github.com/xmos/xcore_sdk/actions/workflows/ci_examples.yml/badge.svg?branch=develop&event=push) |
CI (Linux)       | ![CI](https://github.com/xmos/xcore_sdk/actions/workflows/ci_tests.yml/badge.svg?branch=develop&event=push) |
Docs             | ![CI](https://github.com/xmos/xcore_sdk/actions/workflows/docs.yml/badge.svg?branch=develop&event=push) |

## Cloning

Some dependent components are included as git submodules. These can be obtained by cloning this repository with the following command:

    git clone --recurse-submodules https://github.com/xmos/xcore_sdk.git

## Development Tools

Download and install the xcore [XTC Tools](https://www.xmos.ai/software-tools/) version 15.1.0 or newer. If you already have the XTC Toolchain installed, you can check the version with the following command:

    xcc --version

## Documentation

See the [official documentation](https://www.xmos.ai/documentation/XM-014660-PC-2/html/) for more information including:

- Instructions for installing
- Getting started guides
- Programming tutorials
- API references

## Getting Help

A [Github issue](https://github.com/xmos/xcore_sdk/issues/new/choose) should be the primary method of getting in touch with the XMOS XCORE SDK development team.

## License

This Software is subject to the terms of the [XMOS Public Licence: Version 1](https://github.com/xmos/xcore_sdk/blob/develop/LICENSE.rst). Copyrights and licenses for third party components can be found in [Copyrights and Licenses](https://github.com/xmos/xcore_sdk/blob/develop/doc/copyright.rst).

## Contribution

Contributions are greatly welcomed! For guidelines of contribution please check the [Contributing Guide](https://github.com/xmos/xcore_sdk/blob/develop/doc/contributing.rst).

