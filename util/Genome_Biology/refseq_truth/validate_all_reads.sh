#!/usr/bin/env bash
set -Eeuo pipefail
export LC_ALL=C
export LANG=C

## -----------------------------------------------------------------------------
## Input
## -----------------------------------------------------------------------------

# Argument 1 is a file containing FN/FP, one per line in the format "readID:binID".
# Such a file is produced by compare.sh
INPUT_FILE="$1"

## -----------------------------------------------------------------------------
## Output
## -----------------------------------------------------------------------------

# See _validate_on_existing_index.sh.

## -----------------------------------------------------------------------------
## Script
## -----------------------------------------------------------------------------

# Call _validate_on_existing_index.sh in parallel. With 70 threads.
# Do per line buffering of the stdout/stderr stream.
# Each line in the INPUT_FILE is a argument to call the script with.
parallel --line-buffer -j32 -a "${INPUT_FILE}" $(dirname "$0")/_validate_all_reads.sh
