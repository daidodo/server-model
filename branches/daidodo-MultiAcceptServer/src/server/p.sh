#!/bin/sh

if [ $# -lt 1 ] ; then
  echo "Usage: $0 EXE [EXE2 ...]"
  exit 1
fi

PWD=`pwd`

p_server()
{
  CMD=${PWD}/$1

  PS_RECORD=`ps -ef | grep -v grep | grep "${CMD}"`

  if [ "$PS_RECORD" = "" ]; then
    echo "# ${BIN} is NOT running"
  else
    echo "# ${BIN} is RUNNING"
    echo "${PS_RECORD}"
    exit 0
  fi
}

for BIN in "$*" ; do
  p_server $BIN
done
