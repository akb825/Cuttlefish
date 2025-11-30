#!/bin/sh

command=$1
shift
exitCode=$1
shift

"$command" "$@"
realExitCode=$?
if [ $exitCode != $realExitCode ]; then
	echo "got exit code $realExitCode; expected $exitCode"
	exit 1
fi
