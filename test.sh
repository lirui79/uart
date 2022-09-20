#!/bin/bash

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./

DEVICENAME=$1

echo "$0 $1 $DEVICENAME"

./test_location api.cellocation.com 82 $DEVICENAME  $2