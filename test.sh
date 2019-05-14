#!/bin/bash

try() {
    expected="$1"
    input="$2"

    echo "$input" > tmp
    ./hoc tmp > tmp.s
    gcc -o tmp.out tmp.s
    ./tmp.out
    actual="$?"


    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
        rm tmp tmp.s tmp.out
    else
        echo "$expected expected, but got $actual"
        exit 1
    fi
}

try 0 "return 0;"
try 42 "return 40 + 2;"
try 42 "return 2 + 4 * 10;"
try 21 "return 42 / 2;"
try 15 "return 5*(9-6);"
try 4 "return (3+5)/2;"
try 42 "a = 42; return a;"
try 42 "var = 4; return var * 10 + 2;"
try 0 "return 3 + -3;"
try 0 "return 8 - (3 + 5);"
try 0 "return 15 + (-3*+5);"

echo OK
