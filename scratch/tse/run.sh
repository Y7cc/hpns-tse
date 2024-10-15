#!/bin/bash

# Absolue path to this script
SCRIPT_DIR=$(dirname "$(realpath $0)")
# Absolute paths to useful directories
NS3_DIR="${SCRIPT_DIR:?}"/../../

# DTS Inputs 
# WORKLOAD="${SCRIPT_DIR:?}"/inputs/workload/test/two_comm_node_sndrcv_ring
# WORKLOAD="${SCRIPT_DIR:?}"/inputs/workload/trace/chakra_c
# SYSTEM="${SCRIPT_DIR:?}"/inputs/system/test.json
# COMMUNICATOR="${SCRIPT_DIR:?}"/inputs/communicator/test.json
LOGDIR="${SCRIPT_DIR:?}"/logs
# COMMUNICATOR=\"empty\"

function run {
    cd "${NS3_DIR}"
     mpirun -np 2 ./waf --run "tse \
        --log-dir=${LOGDIR}"
    cd "${SCRIPT_DIR:?}"
}

function debug {
    cd "${NS3_DIR}"
    mpirun -np 2 ./waf configure --enable-mpi --build-profile debug
    ./waf --run="tse" --command-template="gdb --args %s \ 
        --log-dir=${LOGDIR}"
    cd "${SCRIPT_DIR:?}"
}

function cleanup {
    cd "${NS3_DIR}"
    ./waf distclean
    cd "${SCRIPT_DIR:?}"
}

function config {
    cd "${NS3_DIR}"
    ./waf configure --enable-mpi
    cd "${SCRIPT_DIR:?}"
}

cd $NS3_DIR
case "$1" in
-l|--clean)
    cleanup;;
-d|--debug)
    debug;;
-c|--config)
    config;;
-r|--run)
    run;;
-h|--help|*)
    printf "--clean(-l), --debug(-d), --config(-c) or --run(-r)\n";;
esac
