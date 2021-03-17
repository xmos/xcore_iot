# Copyright 2020-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import setuptools

LIB_XCORE_INTERPRETERS = [
    "libs/linux/libxcore_interpreters.so",
    "libs/linux/libxcore_interpreters.so.1.0.1",
    "libs/macos/libxcore_interpreters.dylib",
    "libs/macos/libxcore_interpreters.1.0.1.dylib",
]

EXCLUDES = ["*tests", "*tests.*", "host_library"]

INSTALL_REQUIRES = [
    "numpy==1.19.5",
    "typing-extensions==3.7.4",
]

setuptools.setup(
    name="xcore_interpreters",
    packages=setuptools.find_packages(exclude=EXCLUDES),
    python_requires=">=3.6.8",
    install_requires=INSTALL_REQUIRES,
    package_data={"": LIB_XCORE_INTERPRETERS},
    author="XMOS",
    author_email="support@xmos.com",
    description="XMOS TensorFlow Lite model interpreters.",
    license="LICENSE.txt",
    keywords="xmos xcore",
)
