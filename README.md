AI Deployment Framework
=======================

Summary
-------


Installation
------------

Some dependent libraries are included as git submodules. These can be obtained by cloning this repository with the following command:

    $ git clone git@github.com:xmos/adf.git
    $ git submodule update --init

Install at least version 15 of the XMOS tools from your preferred location and activate it by sourcing `SetEnv` in the installation root.

Install conda on your system if you don't already have it:
https://docs.conda.io/projects/conda/en/latest/user-guide/install/

It is recommended to configure conda with the following options:

    $ conda config --set auto_activate_base false
    $ conda config --set env_prompt '({name})'

[CMake 3.14](https://cmake.org/download/) or newer is required for building libraries and test firmware.  A correct version of CMake is included with the Conda virtual environment (see below).

Virtual Environment
-------------------

It is recommended that you install the virtual environment in the repo's directory:

    $ cd adf
    $ conda env create -p ./adf_venv -f environment.yml

Activate the environment by specifying the path:

    $ conda activate adf_venv/

To remove the environment, deactivate and run:

    $ conda remove -p adf_venv/ --all

Docker Image
------------

The Dockerfile provided is used in the CI system but can serve as a guide to system setup.
Installation of the XMOS tools requires connection to our network.

    $ docker build -t xmos/adf .
    $ docker run -it \
        -v $(pwd):/ws \
        -u $(id -u):$(id -g)  \
        -w /ws  \
        xmos/adf \
        bash -l


Note that this container will stop when you exit the shell
For a persistent container:
 - add "-d" to the docker run command to start detached
 - add "--name somename"
 - enter with "docker exec -it somename bash -l"
 - stop with "docker stop somename"

then inside the container

    # setup environment
    $ conda env create -p adf_venv -f environment.yml
    $ /XMOS/get_tools.py 15.0.1
    # activate tools (each new shell)
    $ conda activate ./adf_venv
    $ module load tools/15.0.1
    # do build
    $ make ci
