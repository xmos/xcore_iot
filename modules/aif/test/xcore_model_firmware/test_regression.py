# Copyright 2020-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import pytest
import yaml
import enum
import math

regression_data = None
report_data = None


def parse_report(content):
    class ParseState(enum.Enum):
        START_TEST = enum.auto()
        ARENA_SIZE = enum.auto()
        CODE_SIZE = enum.auto()
        SRAM_DURATION = enum.auto()
        EXTMEM_DURATION = enum.auto()
        SWMEM_DURATION = enum.auto()

    data = []
    state = None

    lines = content.split("\n")
    for line in lines:
        if line.startswith("START_TEST"):
            state = ParseState.START_TEST
        elif line.startswith("ARENA_SIZE"):
            line_count = 0
            state = ParseState.ARENA_SIZE
        elif line.startswith("CODE_SIZE"):
            line_count = 0
            state = ParseState.CODE_SIZE
        elif line.startswith("SRAM_DURATION"):
            state = ParseState.SRAM_DURATION
        elif line.startswith("EXTMEM_DURATION"):
            state = ParseState.EXTMEM_DURATION
        elif line.startswith("SWMEM_DURATION"):
            state = ParseState.SWMEM_DURATION
        if line.startswith("END_TEST"):
            data.append(
                {
                    "model": model,
                    "par": par,
                    "arena_size": {
                        "SRAM": arena_size_sram,
                        "EXTMEM": arena_size_extmem,
                    },
                    "code_size": {"tile_0": code_size_tile_0,},
                    "benchmark": {
                        "SRAM": benchmark_sram,
                        "EXTMEM": benchmark_extmem,
                        "SWMEM": benchmark_swmem,
                    },
                }
            )

        if state == ParseState.START_TEST:
            model_marker = "model="
            par_marker = "par="
            if line.startswith(model_marker):
                model = line[len(model_marker) :]
            elif line.startswith(par_marker):
                par = int(line[len(par_marker) :])
        elif state == ParseState.ARENA_SIZE:
            size_marker = "  Tensor arena size: "
            if line_count == 5:
                arena_size_sram = int(line[len(size_marker) :].split()[0])
            elif line_count == 10:
                arena_size_extmem = int(line[len(size_marker) :].split()[0])
            line_count += 1
        elif state == ParseState.CODE_SIZE:
            if line_count == 16:
                code_size_tile_0 = int(line.split()[0])
            line_count += 1
        elif state == ParseState.SRAM_DURATION:
            if line.startswith("TOTAL"):
                benchmark_sram = int(line.split()[1])
        elif state == ParseState.EXTMEM_DURATION:
            if line.startswith("TOTAL"):
                benchmark_extmem = int(line.split()[1])
        elif state == ParseState.SWMEM_DURATION:
            if line.startswith("TOTAL"):
                benchmark_swmem = int(line.split()[1])

    return data


def setup_module(module):
    global regression_data
    global report_data

    with open("regression.yml") as fd:
        regression_data = yaml.load(fd.read())

    with open("test.rpt") as fd:
        report_data = parse_report(fd.read())


def test_models():
    def lookup_regression_test(model, par):
        for regression_test in regression_data:
            if regression_test["model"] == model and regression_test["par"] == par:
                return regression_test

    no_errors = True  # until proven otherwise

    for test in report_data:
        model = test["model"]
        par = test["par"]
        print("********************************")
        print(f"Model={model}, Par={par}")
        print("********************************")

        regression = lookup_regression_test(model, par)
        if regression:
            # check code size
            if not math.isclose(
                regression["code_size"]["tile_0"],
                test["code_size"]["tile_0"],
                rel_tol=0.005,
            ):
                print(
                    f'Expected code_size {regression["code_size"]["tile_0"]}, reported code_size={test["code_size"]["tile_0"]}'
                )
                no_errors = False
            # check arena size
            for segment in ["SRAM", "EXTMEM"]:
                if not math.isclose(
                    regression["arena_size"][segment],
                    test["arena_size"][segment],
                    rel_tol=0.005,
                ):
                    print(
                        f'Expected {segment} arena_size {regression["arena_size"][segment]}, reported arena_size={test["arena_size"][segment]}'
                    )
                    no_errors = False
            # check benchmark
            for segment in ["SRAM", "EXTMEM", "SWMEM"]:
                if regression["benchmark"][segment]:
                    if not math.isclose(
                        regression["benchmark"][segment],
                        test["benchmark"][segment],
                        rel_tol=0.005,
                    ):
                        print(
                            f'Expected {segment} benchmark {regression["benchmark"][segment]}, reported benchmark={test["benchmark"][segment]}'
                        )
                        no_errors = False
                else:
                    print(
                        f'Expected {segment} benchmark None, reported benchmark={test["benchmark"][segment]}'
                    )
        else:
            print(f"No regression data")

    assert no_errors


if __name__ == "__main__":
    pytest.main()
