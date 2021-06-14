#!/bin/bash

ls $2 > /dev/null 2>&1
if [ $? -ne 0 ]
  then
  exit 0;
fi

llvm-profdata merge -output=merged.profdata ${@:2}

llvm-cov report -instr-profile=merged.profdata -ignore-filename-regex=\(third_party\|tests\|unit-tf\) -object=$1