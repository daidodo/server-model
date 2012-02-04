#!/bin/bash

if [ $# -lt 2 ] ; then
  echo "Usage: $0 DIR PATTERN_FILE"
  exit 1
fi

BASE_DIR="$1"
PATTERN_FILE="$2"

find $BASE_DIR -type d | grep -v svn | xargs svn ps svn:ignore --file $PATTERN_FILE
