# Copyright (c) 2020, XMOS Ltd, All rights reserved
from __future__ import print_function

import argparse
from pathlib import Path

from convert_tflite_to_c_source import convert_bytes_to_c_source
from tflite2xcore.xcore_model import XCOREModel
from tflite2xcore.xcore_schema import XCOREOpCodes, ExternalOpCodes, BuiltinOpCodes
from tflite2xcore import analyze

SOURCE_DIRECTORY = "src"


def make_operator_code_lut():
    BUILTIN_TPL = "resolver->Add{nm}();"
    CUSTOM_TPL = "resolver->AddCustom(tflite::ops::micro::xcore::{oc}, tflite::ops::micro::xcore::{fun}());"

    operator_code_lut = {
        # XCORE 8-bit
        XCOREOpCodes.XC_lookup_8: CUSTOM_TPL.format(
            oc="Lookup_8_OpCode", fun="Register_Lookup_8"
        ),
        XCOREOpCodes.XC_maxpool2d: CUSTOM_TPL.format(
            oc="MaxPool2D_OpCode", fun="Register_MaxPool2D"
        ),
        XCOREOpCodes.XC_avgpool2d: CUSTOM_TPL.format(
            oc="AvgPool2D_OpCode", fun="Register_AvgPool2D"
        ),
        XCOREOpCodes.XC_avgpool2d_global: CUSTOM_TPL.format(
            oc="AvgPool2D_Global_OpCode", fun="Register_AvgPool2D_Global"
        ),
        XCOREOpCodes.XC_fc: CUSTOM_TPL.format(
            oc="FullyConnected_8_OpCode", fun="Register_FullyConnected_8"
        ),
        XCOREOpCodes.XC_conv2d_shallowin: CUSTOM_TPL.format(
            oc="Conv2D_Shallow_OpCode", fun="Register_Conv2D_Shallow"
        ),
        XCOREOpCodes.XC_conv2d_deep: CUSTOM_TPL.format(
            oc="Conv2D_Deep_OpCode", fun="Register_Conv2D_Deep"
        ),
        XCOREOpCodes.XC_conv2d_1x1: CUSTOM_TPL.format(
            oc="Conv2D_1x1_OpCode", fun="Register_Conv2D_1x1"
        ),
        XCOREOpCodes.XC_conv2d_depthwise: CUSTOM_TPL.format(
            oc="Conv2D_Depthwise_OpCode", fun="Register_Conv2D_Depthwise"
        ),
        XCOREOpCodes.XC_add_8: CUSTOM_TPL.format(
            oc="Add_8_OpCode", fun="Register_Add_8"
        ),
        # XCOREOpCodes.XC_pad: CUSTOM_TPL.format(oc="Pad_OpCode", fun="Register_Pad"),
        # XCORE binarized
        XCOREOpCodes.XC_bsign_8: CUSTOM_TPL.format(
            oc="Bsign_8_OpCode", fun="Register_BSign_8"
        ),
        XCOREOpCodes.XC_bconv2d_bin_DI: CUSTOM_TPL.format(
            oc="BConv2d_Bitpacked_DeepIn_OpCode",
            fun="Register_BConv2D_Bitpacked_Deepin",
        ),
        XCOREOpCodes.XC_bconv2d_bin: CUSTOM_TPL.format(
            oc="BConv2d_Bitpacked_OpCode", fun="Register_BConv2D_Bitpacked"
        ),
    }

    # extend LUT with BuiltIn operators
    for op_code in BuiltinOpCodes:
        nm = "".join(x.capitalize() or "_" for x in op_code.name.split("_"))
        nm = nm.replace("2d", "2D")
        operator_code_lut[op_code] = BUILTIN_TPL.format(nm=nm)

    return operator_code_lut


def make_model_data_filenames(variable_name):
    header_file = Path(SOURCE_DIRECTORY) / f"{variable_name}_model_data.h"
    source_file = Path(SOURCE_DIRECTORY) / f"{variable_name}_model_data.c"

    return header_file, source_file


def make_model_runner_filenames(variable_name):
    header_file = Path(SOURCE_DIRECTORY) / f"{variable_name}_model_runner.h"
    source_file = Path(SOURCE_DIRECTORY) / f"{variable_name}_model_runner.cc"

    return header_file, source_file


def generate_model_data(
    model_path, output_path, variable_name, *, line_width=80, do_analyze=False
):
    header_file_rel, source_file_rel = make_model_data_filenames(variable_name)
    header_file = output_path / header_file_rel
    source_file = output_path / source_file_rel
    source_file.parent.mkdir(exist_ok=True)

    include_guard = variable_name.upper() + "_MODEL_H_"

    with open(model_path, "rb") as model_fd:
        if do_analyze:
            print("Analyzing model:", model_path)
            analyze.print_report(model_path)

        model_data = model_fd.read()

        source, header = convert_bytes_to_c_source(
            data=model_data,
            array_name=variable_name,
            max_line_width=line_width,
            include_guard=include_guard,
        )

        print("Generating header file:", header_file)
        with open(header_file, "w") as header_fd:
            header_fd.write(header)

        print("Generating source file:", source_file)
        with open(source_file, "w") as source_fd:
            source_fd.write(source)


def generate_op_resolver_registrations(model_path, indent=" " * 2):
    operator_count = 0
    operator_registrations = []
    with open(model_path, "rb") as model_fd:

        model_content = model_fd.read()
        model = XCOREModel.deserialize(model_content)
        operator_count = len(model.operator_codes)
        operator_code_lut = make_operator_code_lut()
        for op_code in model.operator_codes:
            registration_line = operator_code_lut.get(
                op_code.code,
                f"// Unable to generate registration code for op_code {op_code.code}",
            )
            operator_registrations.append(f"{indent}{registration_line}")

    return operator_registrations


def generate_model_runner(operator_registrations, output_path, variable_name):
    header_file_rel, source_file_rel = make_model_runner_filenames(variable_name)
    header_file = output_path / header_file_rel
    source_file = output_path / source_file_rel
    source_file.parent.mkdir(exist_ok=True)

    include_guard = variable_name.upper() + "_RUNNER_H_"

    header_template_path = Path(__file__).parent / f"templates/model_runner_header.tpl"
    with open(header_template_path, "r") as header_template_fd:
        header_template = header_template_fd.read()
        header_text = header_template.format(include_guard=include_guard)
        print("Generating header file:", header_file)
        with open(header_file, "w") as header_fd:
            header_fd.write(header_text)

    source_template_path = Path(__file__).parent / f"templates/model_runner_source.tpl"
    with open(source_template_path, "r") as source_template_fd:
        source_template = source_template_fd.read()
        source_text = source_template.format(
            header_file=header_file,
            operator_count=len(operator_registrations),
            operator_registrations="\n".join(operator_registrations),
        )
        print("Generating source file:", source_file)
        with open(source_file, "w") as source_fd:
            source_fd.write(source_text)


def generate_build_files(output_path, variable_name):
    cmake_file = output_path / "CMakeLists.txt"
    _, runner_src = make_model_runner_filenames(variable_name)
    _, array_src = make_model_data_filenames(variable_name)
    include_dir = array_src.parent

    cmake_template_path = Path(__file__).parent / f"templates/project_cmake.tpl"
    with open(cmake_template_path, "r") as cmake_template_fd:
        cmake_template = cmake_template_fd.read()
        cmake_text = cmake_template.format(
            runner_src=runner_src, array_src=array_src, include_dir=include_dir
        )
        print("Generating build file:", cmake_file)
        with open(cmake_file, "w") as cmake_fd:
            cmake_fd.write(cmake_text)


def generate_project(args):
    inputs = args.input
    runner_basename = args.name
    output_path = Path(args.output) / "model_runner"
    print("Generating output path:", output_path)

    # create output_path if it does not exist
    output_path.mkdir(parents=True, exist_ok=True)

    resolver_registrations = set([])

    for i, input_ in enumerate(inputs):
        model_path = Path(input_)
        runner_name = f"{runner_basename}-{i}" if len(inputs) > 1 else runner_basename
        generate_model_data(
            model_path, output_path, runner_name, do_analyze=args.analyze
        )
        resolver_registrations.update(generate_op_resolver_registrations(model_path))

    generate_model_runner(resolver_registrations, output_path, runner_basename)
    generate_build_files(output_path, runner_basename)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description=(
            "Command line tool to convert .tflite file to a model runner project."
        )
    )

    parser.add_argument(
        "--input",
        help="Full filepath of the input TensorFlow Lite file.",
        required=True,
        action="append",
    )

    parser.add_argument(
        "--output",
        help="Full filepath of the output runner project parent directory.",
        default=Path.cwd(),
    )

    parser.add_argument(
        "--analyze",
        action="store_true",
        default=False,
        help="Analyze the output model. "
        "A report is printed showing the runtime memory footprint of the model.",
    )

    parser.add_argument(
        "--name", help="Name to use for the model runner.", default="app",
    )
    args = parser.parse_args()

    generate_project(args)
