#!/usr/bin/bash

# Bash strict mode
set -euo pipefail
IFS=$'\n\t'

input="./data/local/merged_Sliderule/*.csv"

ls -1 ${input} | parallel --verbose --lb --jobs=16 --halt now,fail=1 \
    "build/debug/score --verbose --prediction-label="$1" < {} > predictions/{/.}_score_$1.txt"
echo "Noise" > baseline_$1.txt
./scripts/summarize_scores.sh "./predictions/*_score_$1.txt" 0 >> baseline_$1.txt
echo "Surface" >> baseline_$1.txt
./scripts/summarize_scores.sh "./predictions/*_score_$1.txt" 41 >> baseline_$1.txt
echo "Bathy" >> baseline_$1.txt
./scripts/summarize_scores.sh "./predictions/*_score_$1.txt" 40 >> baseline_$1.txt
