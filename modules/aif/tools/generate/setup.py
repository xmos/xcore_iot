# Copyright 2019-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import setuptools

EXCLUDES = ["templates"]
SCRIPTS = ["convert_tflite_to_c_source.py", "generate_model_runner.py"]

INSTALL_REQUIRES = [
    "jinja2",
]

setuptools.setup(
    name="generate_model",
    packages=setuptools.find_packages(exclude=EXCLUDES),
    scripts=SCRIPTS,
    python_requires=">=3.8.0",
    install_requires=INSTALL_REQUIRES,
    author="XMOS",
    author_email="support@xmos.com",
    description="XMOS Tools to deploy TensorFlow Lite models to xCORE microcontrollers.",
    license="",
    keywords="",
)
