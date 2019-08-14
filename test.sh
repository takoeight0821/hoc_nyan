#!/bin/bash

echo "~~~ gcc ~~~" &&
    gcc -static -o test/tmp.out test/test.c &&
    ./test/tmp.out &&
    gcc -D__hoc__ -E -P test/test.c > test/tmp.c &&
    echo "~~~ generation 0 ~~~" &&
    ./hoc test/tmp.c > test/tmp.s &&
    gcc -static -o test/tmp.out test/tmp.s &&
    ./test/tmp.out &&
    echo "~~~ generation 1 ~~~" &&
    ./gen_first/hoc test/tmp.c > test/tmp_hoc.s &&
    gcc -static -o test/tmp_hoc.out test/tmp_hoc.s &&
    ./test/tmp_hoc.out &&
    echo "~~~ generation 2 ~~~" &&
    ./gen_second/hoc test/tmp.c > test/tmp_hoc.s &&
    gcc -static -o test/tmp_hoc.out test/tmp_hoc.s &&
    ./test/tmp_hoc.out &&
    cmp src/containers.s src/containers_1.s &&
    cmp src/emit.s src/emit_1.s &&
    cmp src/main.s src/main_1.s &&
    cmp src/node.s src/node_1.s &&
    cmp src/parse.s src/parse_1.s &&
    cmp src/sema.s src/sema_1.s &&
    cmp src/token.s src/token_1.s &&
    cmp src/utils.s src/utils_1.s &&
    cmp src/preprocess.s src/preprocess_1.s &&
    rm test/tmp.c test/tmp.s test/tmp.out test/tmp_hoc.s test/tmp_hoc.out &&

    echo OK
