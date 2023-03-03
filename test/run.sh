#!/bin/bash

if [ $# -lt 1 ] ; then
	echo "Usage: ${0} <test_name>"
	exit 1
fi

PATH_DIR=$(dirname $(realpath $0))
PATH_IN=${PATH_DIR}/in
PATH_OUT=${PATH_DIR}/out
PATH_RES=${PATH_DIR}/res.txt
PATH_TIME=${PATH_DIR}/time.txt

IN=${1}
OUT=${PATH_OUT}/$(basename ${IN} .in).out
TEST=$(basename $IN)
RUN=${PATH_DIR}/../tsp
MAX_VALUE=25

${RUN} ${IN} ${MAX_VALUE} 1> ${PATH_RES} 2> ${PATH_TIME}
diff ${PATH_RES} ${OUT} > /dev/null
if [ $? -eq 0 ]; then
    printf "\e[32m[Succ] \e[0m%s \e[33m(%s)\e[0m\n" ${TEST} $(cat ${PATH_TIME})
else
    printf "\e[31m[Fail] \e[0m%s \e[33m(%s)\e[0m\n" ${TEST} $(cat ${PATH_TIME})
fi

rm ${PATH_RES}
rm ${PATH_TIME}