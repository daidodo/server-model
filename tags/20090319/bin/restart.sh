#!/bin/bash

./kill.sh

sleep 1

RET=-1
./start.sh && RET=0

exit $RET
