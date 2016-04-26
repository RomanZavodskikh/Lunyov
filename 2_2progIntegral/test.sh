#!/bin/bash
PREFIX=zrk
for i in `seq 1 $1`
do
    ./a.out $2 > ${PREFIX}$i 
    diff ${PREFIX}$i ${PREFIX}1
    if [ $? -eq 0 ] && [ $i -ne 1 ]
    then
        rm ${PREFIX}$i
    fi
done
