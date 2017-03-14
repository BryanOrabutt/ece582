#!/bin/python
import re

of = open('results.csv', 'w+')
with open('results.txt') as f:
    lines = f.readlines()
    count = 1
    for line in lines:
        try:
            x = str(re.findall('\d+', line)[0])
            of.write(x)
            count = count + 1
            if(count > 32):
                count = 1
                of.write('\n')
            else:
                of.write(',')
        except ValueError:
            continue
        f.close()
of.close()
