#!/usr/bin/env bash

# Run test binaries, passed as arguments

passed=0
total=0
for test_bin in $@; do
    echo "> Running ./$test_bin"
    if ./$test_bin; then
        ((passed += 1))
    fi
    ((total += 1));
done
echo "> Passed: $passed/$total"
if [[ $passed -ne $total ]]; then
    exit 1
fi
exit 0
