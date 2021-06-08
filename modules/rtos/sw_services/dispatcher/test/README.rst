#####################
Dispatcher Unit Tests
#####################

************************
Building & running tests
************************

Run the following commands to build the test firmware:

.. code-block:: console

    $ cmake -B build
    $ cmake --build build --target install
    $ xrun --xscope --args bin/dispatcher_tests.xe -v

## For more unit test options

To run a single test group, run with the `-g` option.

.. code-block:: console

    $ xrun --xscope --args bin/dispatcher_tests.xe -g {group name}

To run a single test, run with the `-g` and `-n` options.

.. code-block:: console

    $ xrun --xscope --args bin/dispatcher_tests.xe -g {group name} -n {test name}

For more unit test options, run with the `-h` option.

.. code-block:: console

    $ xrun --xscope --args bin/dispatcher_tests.xe -h
