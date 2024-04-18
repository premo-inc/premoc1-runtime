#!/bin/bash -e

export PULPRT_TARGET=premoc1
export PULPRUN_TARGET=premoc1

# stdoutをuartに出力する
export io=uart

if [  -n "${ZSH_VERSION:-}" ]; then
        DIR="$(readlink -f -- "${(%):-%x}")"
        scriptDir="$(dirname $DIR)"
else

    scriptDir="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"

fi

source $scriptDir/common.sh
