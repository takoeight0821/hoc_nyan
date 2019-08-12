#!/bin/bash

gcc -E -P test/test.c > test/tmp.c &&
    echo "~~~ generation 0 ~~~" &&
    ./hoc test/tmp.c > test/tmp.s &&
    gcc -static -o test/tmp.out test/tmp.s &&
    ./test/tmp.out &&
    echo "~~~ generation 1 ~~~" &&
    ./gen_first/hoc test/tmp.c > test/tmp_hoc.s &&
    gcc -static -o test/tmp_hoc.out test/tmp_hoc.s &&
    ./test/tmp_hoc.out &&
    rm test/tmp.c test/tmp.s test/tmp.out test/tmp_hoc.s test/tmp_hoc.out &&

    echo OK
