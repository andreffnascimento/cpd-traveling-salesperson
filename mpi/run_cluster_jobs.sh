#!/bin/bash

PROJ_DIR=$CLUSTER_HOME/cpd-traveling-salesperson
PATH_IN=$PROJ_DIR/test/in
PATH_RUN=$PROJ_DIR/mpi/tsp-mpi
OUTPUT_DIR=$PROJ_DIR/results
TIME_DIR=$PROJ_DIR/results
PATH_OUT_1=$PROJ_DIR/test/out/base
PATH_OUT_2=$PROJ_DIR/test/out/inverted


run_tests()
{
    for test in ${PATH_IN}/*.in
    do
        file=$(realpath $test)
        TEST=$(basename $file)
        MAX_VALUE=$(echo ${TEST} | sed -n "s/^.*-\([0-9]*\).*$/\1/p")
        PATH_RES=${OUTPUT_DIR}/results_${TEST}_${nprocs}.txt
        PATH_TIME=${TIME_DIR}/time_${TEST}_${nprocs}.txt
        JOB_NAME="mpi_${nprocs}_${TEST}"
        MEASUREMENTS_FILE=${OUTPUT_DIR}/mpi/measurements_$nprocs.txt
        OUT_1=$PATH_OUT_1/$(basename ${TEST} .in).out
        OUT_2=$PATH_OUT_2/$(basename ${TEST} .in).out

        sbatch --exclude=lab5p[1-20],lab6p[1-9],lab7p[1-9] --time=00:02:30 --ntasks=$nprocs --job-name=$JOB_NAME --output=$PATH_RES --error=$PATH_TIME run_batch.sh $PATH_RUN $file $MAX_VALUE $PATH_RES $PATH_TIME $TEST $MEASUREMENTS_FILE $OUT_1 $OUT_2 
    done
}

execute()
{
    for nprocs in 64 32 16 8 4 2 1
    do
    run_tests
    done
}

execute