#!/bin/bash
make clean && make tests/$1 && tests/$1
