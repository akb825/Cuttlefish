#!/bin/sh

command=$1
args=$2
exitCode=$3

"$command" $args
realExitCode=$?
if [ $exitCode != $realExitCode ]; then
	echo "got exit code $realExitCode; expected $exitCode"
	exit -1
fi
