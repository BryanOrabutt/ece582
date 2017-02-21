#!/bin/python

i = 1
j = 1
k = 1
x = 0

for x in range(0, 64):
    print "%d,%d" % (i*8, j*8)
    j = j + 1
    if j > 8:
        print "%d,%d" % (i*8, 128)
        i = i+1
        j = 1
print "%d,%d" % (128, 128)
