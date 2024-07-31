#!/usr/bin/bash

# Bash strict mode
set -eu
IFS=$'\n\t'

input=$(ls -1 ./data/local/merged_Sliderule/*.csv | tr '\r\n' ' ')

parallel --verbose --lb --jobs 4 --halt now,fail=1 \
    "build/debug/score --verbose \
        --prediction-label={} --ignore-class=41 --class=40 \
        --csv-filename=no_surface_{}.csv \
        ${input} \
        > no_surface_micro_{}.txt" \
    ::: \
    qtrees bathypathfinder medianfilter cshelph coastnet openoceans pointnet
