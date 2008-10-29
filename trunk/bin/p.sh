#!/bin/sh

PWD=`pwd`
BIN=Server.out
CMD=${PWD}/${BIN}

PS_RECORD=`ps -ef | grep -v grep | grep "${BIN}"`

if [ "$PS_RECORD" = "" ]; then
    echo "# ${BIN} is NOT running"
else
    echo "# ${BIN} is RUNNING"
    echo "${PS_RECORD}"
    exit 0
fi