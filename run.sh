#!/usr/bin/env sh
set -e
make clean && make
./cvp -F 16,0,0,0,0 -f 5 -M 0 -A 0 -w 256 -v "${1}"
