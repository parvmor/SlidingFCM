#!/usr/bin/env python3

import sys
if len(sys.argv) != 3:
    print('usage: ./set_diff.py <trace a> <trace b>')
a_pc, a_seq = set([]), set([])
b_pc, b_seq = set([]), set([])
with open(sys.argv[1], 'r') as f:
    tmp = f.readlines()
    for i in tmp:
        pc = int(i.strip().split()[1], base=16)
        seq = int(i.strip().split()[0])
        a_pc.add(pc)
        a_seq.add(seq)
with open(sys.argv[2], 'r') as f:
    tmp = f.readlines()
    for i in tmp:
        pc = int(i.strip().split()[1], base=16)
        seq = int(i.strip().split()[0])
        b_pc.add(pc)
        b_seq.add(seq)
print('seq --- intersection: ' + str(len(a_seq.intersection(b_seq))) + ' a\\b: ' + str(len(a_seq.difference(b_seq))) + ' b\\a: ' + str(len(b_seq.difference(a_seq))))
print('pc --- intersection: ' + str(len(a_pc.intersection(b_pc))) + ' a\\b: ' + str(len(a_pc.difference(b_pc))) + ' b\\a: ' + str(len(b_pc.difference(a_pc))))
