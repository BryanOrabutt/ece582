#!/bin/bash
for j in `seq 1 10`;
do
    for i in `seq 1 32`;
    do
        script -atf -q -c "./montepi -t 1 -r taus_rng_64_states_with_stride_20000000000.dat -t 1000000000 -p ${i}" $1
    done
done
