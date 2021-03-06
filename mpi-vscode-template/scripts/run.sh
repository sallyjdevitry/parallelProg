#!/bin/bash
HEADERS=$(find . -name "*.hpp" -print)
IMPL=$(find . -name "*.cpp" -print)
mpic++ -std=c++11 $HEADERS $IMPL && mpirun -np $1 -oversubscribe ./a.out
rm a.out