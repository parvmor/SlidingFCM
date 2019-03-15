#!/usr/bin/env python3

import os
import subprocess
import sys

ipc = 1.0
trace_cnt = 0

if len(sys.argv) < 2:
    print('usage: ./get_csv.py <dir> [<traces-dir>]')
    sys.exit(1)
elif len(sys.argv) == 2:
    sys.argv.append('./traces')
traces = os.listdir(sys.argv[2])
traces.sort()

for tr in traces:
    print('running: ' + tr + '...')
    f = open('logs/' + tr.split('.')[0])
    output = f.read()
    f.close()
    cur_ipc = float(output.split('\n')[-7].split('=')[1].strip())
    correct_per = float(output.split('\n')[-4].split('(')[-1].split(')')[0][:-1])
    incorrect_per = float(output.split('\n')[-3].split('(')[-1].split(')')[0][:-1])
    with open('results_' + sys.argv[1] + '_' + sys.argv[2].split('/')[-1], 'a+') as f:
        f.write(tr + ',' + str(cur_ipc) + ',' + str(correct_per) + ',' + str(incorrect_per) + '\n')
    ipc, trace_cnt = ipc * cur_ipc, trace_cnt + 1

print(ipc ** (1.0 / trace_cnt))
