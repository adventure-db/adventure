#!/bin/bash

# Run performance tests with debug output off
if [ "${1:(-4)}" = "perf" ]
then
	CARG="-DNDEBUG"
fi

rm tests/$1; make clean && make CARG=$CARG tests/$1 && tests/$1
