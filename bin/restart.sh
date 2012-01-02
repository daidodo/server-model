#!/bin/bash

./kill.sh

sleep 2

RET=-1
./start.sh && RET=0

exit $RET
