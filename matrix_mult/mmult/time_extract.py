#!/bin/python

'''t2 = open('time2k.csv', 'w')
t4 = open('time4k.csv', 'w')
t5 = open('time5k.csv', 'w')

with open('out2k.txt') as f:
    lines = f.readlines()
    for line in lines:
        try:
            x = float(line)
            t2.write(line)
        except ValueError:
            continue
    f.close()
t2.close()

with open('out4k.txt') as f:
    lines = f.readlines()
    for line in lines:
        try:
            x = float(line)
            t4.write(line)
        except ValueError:
            continue
    f.close()
t4.close()

with open('out5k.txt') as f:
    lines = f.readlines()
    for line in lines:
        try:
            x = float(line)
            t5.write(line)
        except ValueError:
            continue
    f.close()
t5.close()'''

t8 = open('ijkcorr.csv', 'w')
with open('ijk-corr.txt') as f:
    lines = f.readlines()
    for line in lines:
        try:
            x = float(line)
            t8.write(line)
        except ValueError:
            continue
    f.close()
t8.close()
