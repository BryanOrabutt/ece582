#!/bin/python

i = 1
j = 1
k = 1
x = 0

for x in range(0, 64):
    print "%d,%d,%d" % (i*32, j*32, k*32)
    k = k+1
    if k > 4:
        k = 1
        j = j+1
        if j > 4:
            j = 1
            i = i+1
