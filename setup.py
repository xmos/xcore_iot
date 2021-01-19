# Copyright (c) 2020, XMOS Ltd, All rights reserved
import setuptools

LIB_XCORE_INTERPRETERS = [
    "libs/linux/libxcore_interpreters.so",
    "libs/linux/libxcore_interpreters.so.1.0.1",
    "libs/macos/libxcore_interpreters.dylib",
    "libs/macos/libxcore_interpreters.1.0.1.dylib",
]

EXCLUDES = ["*tests", "*tests.*", "python_bindings", "xcore_firmware"]

INSTALL_REQUIRES = [
    "numpy==1.19.2",
    "portalocker==2.0.0",
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
    use_scm_version={
        "root": "..",
        "relative_to": __file__,
        "version_scheme": "post-release",
    },
    setup_requires=["setuptools_scm"],
)
