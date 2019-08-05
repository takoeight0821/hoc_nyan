#!/bin/bash

gcc -E -P test/test.c > test/tmp.c &&
    ./hoc test/tmp.c > test/tmp.s &&
    gcc -static -o test/tmp.out test/tmp.s &&
    ./test/tmp.out &&
    rm test/tmp.c test/tmp.s test/tmp.out &&

    echo OK
