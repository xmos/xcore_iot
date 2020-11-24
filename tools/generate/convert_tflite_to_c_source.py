# Copyright (c) 2020, XMOS Ltd, All rights reserved

from __future__ import print_function

import sys
import argparse
from pathlib import Path


def convert_bytes_to_c_source(data, array_name, max_line_width, include_guard):
    """Returns strings representing a C constant array containing `data`.
  """

    def data_to_array_values(data):
        starting_pad = "   "
        array_lines = []
        array_line = starting_pad
        for value in bytearray(data):
            if (len(array_line) + 4) > max_line_width:
                array_lines.append(array_line + "\n")
                array_line = starting_pad
            array_line += " 0x%02x," % (value)
        if len(array_line) > len(starting_pad):
            array_lines.append(array_line + "\n")
        return "".join(array_lines)

    source_template_path = Path(__file__).parent / f"templates/model_data_source.tpl"
    with open(source_template_path, "r") as source_template_fd:
        source_template = source_template_fd.read()
        source_text = source_template.format(
            array_name=array_name,
            array_length=len(data),
            array_values=data_to_array_values(data),
        )

    header_template_path = Path(__file__).parent / f"templates/model_data_header.tpl"
    with open(header_template_path, "r") as header_template_fd:
        header_template = header_template_fd.read()
        header_text = header_template.format(
            include_guard=include_guard, array_name=array_name, array_length=len(data),
        )

    return source_text, header_text


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description=(
            "Command line tool to convert .tflite file to C source code files."
        )
    )

    parser.add_argument(
        "--input",
        help="Full filepath of the input TensorFlow Lite file.",
        required=True,
    )

    parser.add_argument(
        "--variable-name", help="Name to use for the C data array variable.",
    )

    parser.add_argument(
        "--source", help="Full filepath of the output C source file.",
    )

    parser.add_argument(
        "--header", help="Full filepath of the output C header file.",
    )

    parser.add_argument(
        "--include-guard", help="Name to use for the C header include guard."
    )

    parser.add_argument(
        "--line-width", type=int, help="Width to use for formatting.", default=80
    )

    args = parser.parse_args()

    # setup defaults
    variable_name = args.variable_name or "tflite_model"
    source_file = args.source or "{variable_name}.c".format(variable_name=variable_name)
    header_file = args.header or "{variable_name}.h".format(variable_name=variable_name)
    include_guard = (
        args.include_guard
        or "{variable_name}_H_".format(variable_name=variable_name).upper()
    )

    with open(args.input, "rb") as input_fd:
        input_data = input_fd.read()

    source, header = convert_bytes_to_c_source(
        data=input_data,
        array_name=variable_name,
        max_line_width=args.line_width,
        include_guard=include_guard,
    )

    with open(source_file, "w") as source_fd:
        source_fd.write(source)

    with open(args.header, "w") as header_fd:
        header_fd.write(header)
