#!/bin/bash

PWD=`pwd`
BIN=Server.out
CMD=${PWD}/${BIN}

PS_RECORD=`ps -ef | grep -v grep | grep "${BIN}"`
PS_PID=`echo "${PS_RECORD}" | gawk '{print $2}'`

if [ "${PS_RECORD}" = "" ]; then
    echo "# ${BIN} is NOT running"
else
    kill -9 ${PS_PID}
    
    sleep 1

    PS_CHECK=`ps -ef | grep -v grep | grep "${BIN}"`
    if [ "${PS_CHECK}" = "" ]; then
        echo "# ${BIN} stopped"
        echo "${PS_RECORD}"
        exit 0
    else
        echo "# killing ${BIN} FAILED"
        echo "${PS_CHECK}"
        exit -1
    fi
fi