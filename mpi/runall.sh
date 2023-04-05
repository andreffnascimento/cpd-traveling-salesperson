#!/bin/bash

if [ $# -lt 1 ] ; then
	echo "Usage: sh ${0} <num processes>"
	exit 1
fi

PATH_DIR=$(dirname $(realpath $0))
PATH_RUN=${PATH_DIR}/run.sh
PATH_IN="test/in"

make clean 
make build MACROS=

printf "\t\e[33mRunning the test suit..\n\e[0m"
for test in ${PATH_IN}/*.in
do
	file=$(realpath $test)
	${PATH_RUN} ${file} $1
done
