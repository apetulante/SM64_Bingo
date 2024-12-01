#!/bin/bash
set -e

project_root="$(cd "$(dirname "$0")" && pwd -P)"

pushd $project_root
    # ~/bin/ntime python3 make_board.py data/16Star.txt

    mkdir -p build
    clang -O2 make_board.c -DDEBUG=1 -o build/make_board
    ~/bin/ntime ./build/make_board data/16Star.txt
popd