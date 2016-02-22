#!/bin/bash
PREFIX=zrk
FILE=/bin/bash
TIMES=40
for chnum in `seq 9`
do
    for i in `seq $TIMES`
    do
        echo $chnum:$i
        strace ./a.out $chnum $FILE > ${PREFIX}${chnum}_$i 2>err${chnum}_$i
        diff ${PREFIX}${chnum}_$i $FILE 
        if [ $? -eq 0 ]
        then
            rm ${PREFIX}${chnum}_$i err${chnum}_$i
        fi
    done
done
