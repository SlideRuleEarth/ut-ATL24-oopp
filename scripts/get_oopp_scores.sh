#!/usr/bin/bash

# Bash strict mode
set -eu
IFS=$'\n\t'

input=$(ls -1 ./predictions/*_classified.csv)

build/debug/score --verbose \
    --ignore-class=41 --class=40 \
    --csv-filename=no_surface_oopp.csv \
    ${input} \
    > no_surface_micro_oopp.txt

build/debug/score --verbose \
    --csv-filename=oopp.csv \
    ${input} \
    > micro_oopp.txt
