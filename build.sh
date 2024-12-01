#!/bin/bash
set -e

project_root="$(cd "$(dirname "$0")" && pwd -P)"

pushd $project_root
    python3 make_board.py data/16Star.txt
popd