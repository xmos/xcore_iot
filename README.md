# XCore SDK Repository

The XCore SDK is comprised of the following components:

- C Libraries - Libraries to support programming for XCore in the C programming language.
- Code Examples - Examples showing a variety of XCore features based on bare-metal and FreeRTOS programming.
- FreeRTOS - Libraries to support FreeRTOS operation on xcore.ai.
- Documentation - Getting started guides, references and API guides.

The SDK is designed to be used in conjunction with the xcore.ai Explorer board. The example applications compile targeting this board. Further information about the Explorer board, and xcore.ai device are available to on [www.xmos.ai](https://www.xmos.ai/).

## Build Status

Build Type       |    Status     |
-----------      | --------------|
CI (Linux)       | ![CI](https://github.com/xmos/xcore_sdk/actions/workflows/ci.yml/badge.svg?branch=develop&event=push) |
Docs             | ![CI](https://github.com/xmos/xcore_sdk/actions/workflows/docs.yml/badge.svg?branch=develop&event=push) |

## Cloning

Some dependent components are included as git submodules. These can be obtained by cloning this repository with the following command:

    $ git clone --recurse-submodules https://github.com/xmos/xcore_sdk.git

## Documentation

See the [official documentation](https://www.xmos.ai/xcore-sdk/) for more information including:

- Instructions for installing
- Programming tutorials
- How to build and run example applications
- API references

# Getting Help

A [Github issue](https://github.com/xmos/xcore_sdk/issues/new/choose) should be the primary method of getting in touch with the XMOS SDK development team.

## License

This Software is subject to the terms of the [XMOS Public Licence: Version 1](https://github.com/xmos/xcore_sdk/blob/develop/LICENSE.rst)
