#!/bin/bash
set -e

git submodule foreach --quiet 'echo $name `git describe --all --contains --always`'

