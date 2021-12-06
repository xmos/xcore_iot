import pytest
import Pyxsim as px
from typing import Mapping

# from https://github.com/pytest-dev/pytest/issues/3730#issuecomment-567142496
def pytest_configure(config):
    config.addinivalue_line(
        "markers", "uncollect_if(*, func): function to unselect tests from parametrization"
    )

def pytest_collection_modifyitems(config, items):
    removed = []
    kept = []
    for item in items:
        m = item.get_closest_marker('uncollect_if')
        if m:
            func = m.kwargs['func']
            if func(**item.callspec.params):
                removed.append(item)
                continue
        kept.append(item)
    if removed:
        config.hook.pytest_deselected(items=removed)
        items[:] = kept

def pytest_addoption(parser):
    parser.addoption("--nightly", action="store_true")

@pytest.fixture
def build():
    def _builder(directory: str, env: Mapping = None, bin_child: str = ""):
        if bin_child and not bin_child.endswith("/"):
            bin_child += "/"
        px.cmake_build(directory, bin_child, env)
    yield _builder

@pytest.fixture
def nightly(pytestconfig):
    return pytestconfig.getoption("nightly")