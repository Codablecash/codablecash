#!/bin/bash -eux


pushd target

make all -j6

./testall -v -g TestAllocationIfGroup
make report

popd


