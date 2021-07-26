FROM ghcr.io/xmos/xcore_builder:main

# This Dockerfile is for use by the XMOS XCore SDK CI system
# It provides a minimal environment needed to build applications and run tests

ARG CMAKE_VERSION=3.14.7

# install cmake
RUN wget -O- https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-Linux-x86_64.tar.gz | tar -xz -C /opt
RUN echo "export PATH=/opt/cmake-3.14.7-Linux-x86_64/bin:$PATH" \
    > /etc/profile.d/cmake.sh

# set login shell
SHELL ["/bin/bash", "-l", "-c"]
