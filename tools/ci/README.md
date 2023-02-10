# Continuous Integration Tools

## Running Source Checks

Source and license checks must currently be performed manually.  The `infr` tools must be installed first.

To install te `infr` tools, run:

    pip install --src infr -r infr_requirements.txt

### Source Check

First copy the ignore list to the root of the repository

    cp tools/ci/.xmos_ignore_source_check {path to root of repository}

To run the check

    xmos_source_check check {path to root of repository} xmos_public_v1

To run the update 

    xmos_source_check update {path to root of repository} xmos_public_v1

### License Check

To run the check

    xmos_license_check check {path to root of repository} xmos_public_v1
