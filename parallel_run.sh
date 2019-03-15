#!/usr/bin/env zsh

cores=${1}
ls $CVPTRACES/*/*.gz | xargs -I{} -P${cores} bash -c 'echo {} && ./cvp -F 16,0,0,0,0 -f 5 -M 0 -A 0 -w 256 -v {} 1> ./logs/$(basename {} .gz) 2>/dev/null'
