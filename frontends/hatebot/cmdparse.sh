#!/bin/bash
CMD=$1
shift
commands/$CMD $* 2> /dev/null

