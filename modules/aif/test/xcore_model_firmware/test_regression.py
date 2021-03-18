# Copyright 2020-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import pytest
import yaml


def test_cifar10():
    with open("regression.yml") as fd:
        bits = yaml.load(fd.read())
        print(bits)
        assert 1 == 3


if __name__ == "__main__":
    pytest.main()
