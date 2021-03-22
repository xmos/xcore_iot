# Copyright 2020-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import pytest
import yaml

regression_data = None


def setup_module(module):
    global regression_data
    with open("regression.yml") as fd:
        regression_data = yaml.load(fd.read())


def test_cifar10():
    print(regression_data)
    assert 1 == 3


if __name__ == "__main__":
    pytest.main()
