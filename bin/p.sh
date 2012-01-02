#!/bin/sh

PWD=`pwd`
BIN=server.out
CMD=${PWD}/${BIN}

PS_RECORD=`ps -ef | grep -v grep | grep "${CMD}"`

if [ "$PS_RECORD" = "" ]; then
    echo "# ${BIN} is NOT running"
else
    echo "# ${BIN} is RUNNING"
    echo "${PS_RECORD}"
    exit 0
fi
