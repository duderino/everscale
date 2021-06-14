#!/bin/bash

llvm-profdata merge -output=merged.profdata tests/*.profraw

llvm-cov report -instr-profile=merged.profdata -ignore-filename-regex=\(third_party\|tests\|unit-tf\) -object=$1