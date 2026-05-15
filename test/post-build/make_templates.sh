#!/bin/bash
# Usage: ./make_templates.sh <path-to-opentrim-binary>
# Creates template output files out{N}j{J}.h5 for all test cases.

set -e

OPENTRIM=${1:?Usage: $0 <path-to-opentrim-binary>}
SCRIPT_DIR=$(dirname "$(realpath "$0")")

cd "$SCRIPT_DIR"

for N in 1 2 3; do
    for J in 1 2; do
        echo "Running: in${N}.json  -j ${J}  ->  out${N}j${J}.h5"
        "$OPENTRIM" -f "in${N}.json" -j "$J" -o "out${N}j${J}"
    done
done

echo "Done. Template files created in $SCRIPT_DIR"
