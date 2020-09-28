# Contributing Guidelines & Tips

## Pull Request Checklist
Before sending your pull requests, make sure you followed this list.

* Read [contributing guidelines and standards](CONTRIBUTING.md).
* Ensure all example applications [build](#Building-Examples).
* Run [tests](#Running-Tests).

## How to Contribute

### Contribution Guidelines and Standards

Before sending your pull request, make sure your changes are consistent with these guidelines and are consistent with the coding style used in this ai_tools repository.

#### General Guidelines and Philosophy For Contribution

* Include unit tests when you contribute new features, as they help to a) prove that your code works correctly, and b) guard against future breaking changes to lower the maintenance cost.
* Bug fixes also generally require unit tests, because the presence of bugs usually indicates insufficient test coverage.
* Keep API compatibility in mind when you change code.

#### Python coding style

All python code should be [`blackened`](https://black.readthedocs.io/en/stable/).
For convenience, the default workspace settings file under `.vscode/` enables format-on-save, and `black` is also provided in the conda environments.

#### C, xC and ASM coding style

Changes to C, xC or ASM should be consistent with the style of existing C, xC and ASM code.

#### C++ coding style

Changes to C++ code should conform to
[Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).

Use `clang-tidy` to check your C/C++ changes. To install `clang-tidy` on ubuntu:16.04, do:

```bash
$ apt-get install -y clang-tidy
```

You can check a C/C++ file by doing:


```bash
$ clang-format <my_cc_file> --style=google > /tmp/my_cc_file.cc
$ diff <my_cc_file> /tmp/my_cc_file.cc
```

### Building Examples

A script is provided to build all the example applications.  Run this script with:

```bash
$ ./build_examples.sh
```

### Running Tests

A script is provided to run all the tests on a connected xcore.ai device.  Run this script with:

```bash
$ ./run_tests.sh
```

## Development Tips

### git Submodules

At times submodule repositories will need to be updated.  To update all submodules, run the following command

```bash
$ git submodule update --init --recursive
```
