#!/bin/bash

echo "~~~ gcc ~~~" &&
    gcc -I./include -static -o test/tmp.out test/test.c &&
    ./test/tmp.out &&
    gcc -I./include -D__hoc__ -E -P test/test.c > test/tmp.c &&
    echo "~~~ generation 0 ~~~" &&
    ./hoc test/tmp.c > test/tmp.s &&
    gcc -static -o test/tmp.out test/tmp.s &&
    ./test/tmp.out &&
    echo "~~~ generation 1 ~~~" &&
    ./build/g1/hoc test/tmp.c > test/tmp_hoc.s &&
    gcc -static -o test/tmp_hoc.out test/tmp_hoc.s &&
    ./test/tmp_hoc.out &&
    echo "~~~ generation 2 ~~~" &&
    ./build/g2/hoc test/tmp.c > test/tmp_hoc.s &&
    gcc -static -o test/tmp_hoc.out test/tmp_hoc.s &&
    ./test/tmp_hoc.out &&
    cmp build/g1/containers.s build/g2/containers.s &&
    cmp build/g1/emit.s build/g2/emit.s &&
    cmp build/g1/main.s build/g2/main.s &&
    cmp build/g1/node.s build/g2/node.s &&
    cmp build/g1/parse.s build/g2/parse.s &&
    cmp build/g1/sema.s build/g2/sema.s &&
    cmp build/g1/token.s build/g2/token.s &&
    cmp build/g1/utils.s build/g2/utils.s &&
    cmp build/g1/cpp.s build/g2/cpp.s &&
    rm test/tmp.c test/tmp.s test/tmp.out test/tmp_hoc.s test/tmp_hoc.out &&

    echo OK
