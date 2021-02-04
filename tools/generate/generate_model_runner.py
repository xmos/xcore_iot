# Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
# XMOS Public License: Version 1
from __future__ import print_function

import argparse
from pathlib import Path

import jinja2

from convert_tflite_to_c_source import convert_bytes_to_c_source
from tflite2xcore.xcore_model import XCOREModel
from tflite2xcore.xcore_schema import XCOREOpCodes, ExternalOpCodes, BuiltinOpCodes
from tflite2xcore import analyze


def get_template(filename):
    jinja_env = jinja2.Environment(
        loader=jinja2.FileSystemLoader(searchpath=Path(__file__).parent / "templates")
    )

    return jinja_env.get_template(filename)


def make_operator_code_lut():
    custom_operator_lut = {
        # XCORE 8-bit
        XCOREOpCodes.XC_lookup_8: ("Lookup_8_OpCode", "Register_Lookup_8"),
        XCOREOpCodes.XC_maxpool2d: ("MaxPool2D_OpCode", "Register_MaxPool2D"),
        XCOREOpCodes.XC_avgpool2d: ("AvgPool2D_OpCode", "Register_AvgPool2D"),
        XCOREOpCodes.XC_avgpool2d_global: (
            "AvgPool2D_Global_OpCode",
            "Register_AvgPool2D_Global",
        ),
        XCOREOpCodes.XC_fc: ("FullyConnected_8_OpCode", "Register_FullyConnected_8"),
        XCOREOpCodes.XC_conv2d_shallowin: (
            "Conv2D_Shallow_OpCode",
            "Register_Conv2D_Shallow",
        ),
        XCOREOpCodes.XC_conv2d_deep: ("Conv2D_Deep_OpCode", "Register_Conv2D_Deep"),
        XCOREOpCodes.XC_conv2d_1x1: ("Conv2D_1x1_OpCode", "Register_Conv2D_1x1"),
        XCOREOpCodes.XC_conv2d_depthwise: (
            "Conv2D_Depthwise_OpCode",
            "Register_Conv2D_Depthwise",
        ),
        XCOREOpCodes.XC_add_8: ("Add_8_OpCode", "Register_Add_8"),
        # XCOREOpCodes.XC_pad: ("Pad_OpCode", "Register_Pad"),
        # XCORE binarized
        XCOREOpCodes.XC_bsign_8: ("Bsign_8_OpCode", "Register_BSign_8"),
        XCOREOpCodes.XC_bconv2d_bin_DI: (
            "BConv2d_Bitpacked_DeepIn_OpCode",
            "Register_BConv2D_Bitpacked_Deepin",
        ),
        XCOREOpCodes.XC_bconv2d_bin: (
            "BConv2d_Bitpacked_OpCode",
            "Register_BConv2D_Bitpacked",
        ),
    }

    # LUT for BuiltIn operators
    builtin_operator_lut = {}
    for op_code in BuiltinOpCodes:
        nm = "".join(x.capitalize() or "_" for x in op_code.name.split("_"))
        nm = nm.replace("2d", "2D")
        builtin_operator_lut[op_code] = nm

    return builtin_operator_lut, custom_operator_lut


def make_model_data_filenames(name):
    header_file = Path(f"{name}_model_data.h")
    source_file = Path(f"{name}_model_data.c")

    return header_file, source_file


def make_model_runner_filenames(name):
    header_file = Path(f"{name}_model_runner.h")
    source_file = Path(f"{name}_model_runner.cc")

    return header_file, source_file


def generate_model_data(
    model_path, output_path, variable_name, *, line_width=80, do_analyze=False
):
    header_file_rel, source_file_rel = make_model_data_filenames(variable_name)
    header_file = output_path / header_file_rel
    source_file = output_path / source_file_rel
    source_file.parent.mkdir(exist_ok=True)

    include_guard = variable_name.upper() + "_MODEL_DATA_H_"

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


def get_model_information(model_path):
    builtin_operators = set([])
    custom_operators = set([])
    unknown_operators = set([])

    with open(model_path, "rb") as model_fd:
        model_content = model_fd.read()
        model = XCOREModel.deserialize(model_content)

        layer_count = len(model.subgraphs[0].operators)

        builtin_operator_lut, custom_operator_lut = make_operator_code_lut()
        for op_code in model.operator_codes:
            if op_code.code in builtin_operator_lut:
                builtin_operators.add(builtin_operator_lut[op_code.code])
            elif op_code.code in custom_operator_lut:
                custom_operators.add(custom_operator_lut[op_code.code])
            else:
                unknown_operators.add(op_code.code)

    return layer_count, builtin_operators, custom_operators, unknown_operators


def generate_model_runner(layer_count, operator_registrations, output_path, name):
    header_file_rel, source_file_rel = make_model_runner_filenames(name)
    header_file = output_path / header_file_rel
    source_file = output_path / source_file_rel
    source_file.parent.mkdir(exist_ok=True)

    include_guard = name.upper() + "_MODEL_RUNNER_H_"

    print("Generating header file:", header_file)
    header_template = get_template("model_runner_header.jinja2")
    header_text = header_template.render({"include_guard": include_guard, "name": name})
    with open(header_file, "w") as header_fd:
        header_fd.write(header_text)

    print("Generating source file:", source_file)
    source_template = get_template("model_runner_source.jinja2")
    source_text = source_template.render(
        {
            "header_file": header_file_rel.name,
            "name": name,
            "layer_count": layer_count,
            "builtin_operators": operator_registrations["builtin_operators"],
            "custom_operators": operator_registrations["custom_operators"],
            "unknown_operators": operator_registrations["unknown_operators"],
        }
    )
    with open(source_file, "w") as source_fd:
        source_fd.write(source_text)


def generate_project(inputs, runner_basename, output, *, do_analyze=False):
    output_path = Path(output)
    print("Generating output path:", output_path)

    # create output_path if it does not exist
    output_path.mkdir(parents=True, exist_ok=True)

    layer_count = 0
    operator_registrations = {
        "builtin_operators": set([]),
        "custom_operators": set([]),
        "unknown_operators": set([]),
    }

    for i, input_ in enumerate(inputs):
        model_path = Path(input_)
        runner_name = f"{runner_basename}_{i}" if len(inputs) > 1 else runner_basename
        generate_model_data(model_path, output_path, runner_name, do_analyze=do_analyze)
        (
            model_layer_count,
            builtin_operators,
            custom_operators,
            unknown_operators,
        ) = get_model_information(model_path)
        layer_count = max(layer_count, model_layer_count)
        operator_registrations["builtin_operators"].update(builtin_operators)
        operator_registrations["custom_operators"].update(custom_operators)
        operator_registrations["unknown_operators"].update(unknown_operators)

    generate_model_runner(
        model_layer_count, operator_registrations, output_path, runner_basename
    )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description=(
            "Command line tool to generate a model runner project for given .tflite model file(s)."
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
        help="Full filepath of the output directory where source files will be generated.",
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

    generate_project(args.input, args.name, args.output, do_analyze=args.analyze)
