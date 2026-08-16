#!/usr/bin/env bash

# Run from the project's root directory
cd "$(git rev-parse --show-toplevel)"

find . \
     -type d \( -name .git -o -name build \) -prune\
  -o -type f \( -name '*.c' -o -name '*.h' \) -print \
| xargs ctags --kinds-C='*' -e

(( $? == 0 )) && echo "Generating Tags: Done!" || echo "Generating Tags: Failed!"
