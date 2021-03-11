# Running Source Checks

First copy the ignore list to the root of the repository

    $ cp tools/ci/.xmos_ignore_source_check .

To run the check

    $ xmos_source_check check . xmos_public_v1

To run the update 

    $ xmos_source_check update . xmos_public_v1
