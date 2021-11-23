import pytest
import Pyxsim as px
from typing import Mapping

def pytest_addoption(parser):
    parser.addoption("--nightly", action="store_true")

@pytest.fixture
def build():
    def _builder(directory: str, env: Mapping, bin_child: str =""):
        if bin_child and not bin_child.endswith("/"):
            bin_child += "/"
        px.cmake_build(directory, bin_child, env)
    yield _builder

@pytest.fixture
def nightly(pytestconfig):
    return pytestconfig.getoption("nightly")