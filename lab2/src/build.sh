#!/bin/bash

echo "=== Building static library ==="
gcc -c revert_string.c -o revert_string.o
ar rcs librevert.a revert_string.o
gcc main.c -L. -lrevert -o static_program
echo "Static program built: static_program"

echo "=== Building dynamic library ==="
gcc -c -fPIC revert_string.c -o revert_string_dynamic.o
gcc -shared -o librevert.so revert_string_dynamic.o
gcc main.c -L. -lrevert -o dynamic_program
echo "Dynamic program built: dynamic_program"

echo "=== Testing programs ==="
echo "Static library test:"
./static_program "Hello"

echo "Dynamic library test:"
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
./dynamic_program "World"