#!/bin/bash

try() {
    expected="$1"
    input="$2"

    echo "$input" > tmp
    ./hoc tmp > tmp.s
    gcc -c test.c -o test.o
    gcc -o tmp.out tmp.s test.o
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

try 0 "main () {return 0;}"
try 42 "main () {return 40 + 2;}"
try 42 "main () {return 2 + 4 * 10;}"
try 21 "main () {return 42 / 2;}"
try 15 "main () {return 5*(9-6);}"
try 4 "main () {return (3+5)/2;}"
try 42 "main () { a = 42; return a; }"
try 42 "main () {var = 4; return var * 10 + 2;}"
try 0 "main () {return 3 + -3;}"
try 0 "main () {return 8 - (3 + 5);}"
try 0 "main () {return 15 + (-3*+5);}"
try 1 "main () {return 1 <= 1;}"
try 1 "main () {return 42 == 4 * 10 + 2;}"
try 1 "main () {return 1 < 2;}"
try 1 "main () {return 2 > 1;}"
try 1 "main () {if (1) return 1;}"
try 2 "main () {if (0) return 1; else return 2;}"
try 3 "main () { 1; 2; return 3; }"
try 1 "main () {if (0) { return 0; } else { return 1; }}"
try 0 "main () { f(); return 0; }"
try 0 "main () { return g(0); }"
try 1 "id(x) { return x; }main() { return id(1); }"
try 0 "main () { while (0) { return 1; } return 0; }"
try 55 "main () { x = 10; sum = 0; while (x != 0) { sum = sum + x; x = x - 1; } return sum; }"

echo OK
