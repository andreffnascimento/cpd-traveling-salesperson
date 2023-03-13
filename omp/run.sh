#!/bin/bash

if [ $# -lt 1 ] ; then
	echo "Usage: ${0} <test_name>"
	exit 1
fi

PATH_DIR=$(dirname $(realpath $0))
PATH_TEST=${PATH_DIR}/../test
PATH_IN=${PATH_TEST}/in
PATH_OUT_1=${PATH_TEST}/out/base
PATH_OUT_2=${PATH_TEST}/out/inverted
PATH_RES=${PATH_DIR}/bin/res.txt
PATH_TIME=${PATH_DIR}/bin/time.txt

IN=${1}
OUT_1=${PATH_OUT_1}/$(basename ${IN} .in).out
OUT_2=${PATH_OUT_2}/$(basename ${IN} .in).out
TEST=$(basename $IN)
RUN=${PATH_DIR}/tsp
MAX_VALUE=$(echo ${TEST} | sed -n "s/^.*-\([0-9]*\).*$/\1/p")

${RUN} ${IN} ${MAX_VALUE} 1> ${PATH_RES} 2> ${PATH_TIME}
diff ${PATH_RES} ${OUT_1} > /dev/null
if [ $? -eq 0 ]; then
    printf "\e[32m[Succ] \e[0m%s \e[33m(%s)\e[0m\n" ${TEST} $(cat ${PATH_TIME})
    rm ${PATH_RES}
    rm ${PATH_TIME}
    exit 0
fi

diff ${PATH_RES} ${OUT_2} > /dev/null
if [ $? -eq 0 ]; then
    printf "\e[32m[Succ] \e[0m%s \e[33m(%s)\e[0m\n" ${TEST} $(cat ${PATH_TIME})
else
    printf "\e[31m[Fail] \e[0m%s \e[33m(%s)\e[0m\n" ${TEST} $(cat ${PATH_TIME})
fi

rm ${PATH_RES}
rm ${PATH_TIME}
