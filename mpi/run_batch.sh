#!/bin/bash

PATH_RUN=$1
file=$2
MAX_VALUE=$3
PATH_RES=$4
PATH_TIME=$5
TEST=$6
MEASUREMENTS_FILE=$7
OUT_1=$8
OUT_2=$9

srun --cpus-per-task=4 ${PATH_RUN} ${file} ${MAX_VALUE}

touch $MEASUREMENTS_FILE

diff ${PATH_RES} ${OUT_1} >/dev/null
if [ $? -eq 0 ]; then
    echo [Succ] ${TEST} $(cat ${PATH_TIME}) >> $MEASUREMENTS_FILE
    exit 0
fi

diff ${PATH_RES} ${OUT_2} > /dev/null
if [ $? -eq 0 ]; then
    echo [Succ] ${TEST} $(cat ${PATH_TIME}) >> $MEASUREMENTS_FILE
else
    echo [Fail] ${TEST} $(cat ${PATH_TIME}) >> $MEASUREMENTS_FILE
fi