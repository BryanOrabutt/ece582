#!/bin/python
import re
import sys

minthread = int(sys.argv[1])
maxthread = int(sys.argv[2])
ifname = str(sys.argv[3])

of = open('results.csv', 'w+')
with open(ifname) as f:
    lines = f.readlines()
    count = minthread
    for line in lines:
        try:
            x = str(re.findall('\d+.\d+', line)[0])
            of.write(x)
            count = count + 1
            if(maxthread):
                count = minthread
                of.write('\n')
            else:
                of.write(',')
        except ValueError:
            continue
        f.close()
of.close()
