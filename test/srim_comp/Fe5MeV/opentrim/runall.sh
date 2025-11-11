#!/usr/bin/env bash

# Loop over test cases 
for i in {8..9}; do
    # Build the filename – adjust the prefix/suffix as needed
    filename="t${i}.json"

    # Check if the file exists (optional but often useful)
    if [[ -e "$filename" ]]; then
        opentrim -n 10000 -f $filename
    fi
done