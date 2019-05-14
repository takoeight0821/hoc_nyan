#!/bin/bash

try() {
    expected="$1"
    input="$2"

    echo "$input" > tmp
    ./hoc tmp > tmp.s
    gcc -o tmp.out tmp.s
    ./tmp.out
    actual="$?"

    rm tmp tmp.s tmp.out

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$expected expected, but got $actual"
        exit 1
    fi
}

try 0 "return 0;"
try 42 "return 40 + 2;"
try 42 "return 2 + 4 * 10;"

echo OK
