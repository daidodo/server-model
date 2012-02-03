#!/bin/sh

PWD=`pwd`
BIN=server.out
CMD=${PWD}/${BIN}
PS_RECORD=`ps -ef | grep -v grep | grep "${CMD}"`

if [ "${PS_RECORD}" != "" ]; then
    echo "# ${BIN} is RUNNING"
    echo "${PS_RECORD}"
    exit 0
else
    export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:../lib/
    ulimit -n 100000
    ulimit -s unlimited
    ulimit -c unlimited
#memory leak check
    #valgrind --tool=memcheck --leak-check=full ${CMD} 1>/dev/null 2>memcheck.txt &
#thread synchronisation check
    #valgrind --tool=helgrind --trace-level=1 ${CMD} 1>/dev/null 2>syncheck.txt &
#run program
    ${CMD} 1>/dev/null &

    sleep 1

    PS_CHECK=`ps -ef | grep -v grep | grep "${CMD}"`
    if [ "${PS_CHECK}" = "" ]; then
        echo "# starting ${BIN} FAILED"
        exit -1
    else
        echo "# ${BIN} started"
        echo "${PS_CHECK}"
        exit 0
    fi
fi
