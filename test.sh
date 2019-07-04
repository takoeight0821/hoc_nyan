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

try 0 "int main () {return 0;}"
try 42 "int main () {return 40 + 2;}"
try 42 "int main () {return 2 + 4 * 10;}"
try 21 "int main () {return 42 / 2;}"
try 15 "int main () {return 5*(9-6);}"
try 4 "int main () {return (3+5)/2;}"
try 42 "int main () { int a; a = 42; return a; }"
try 42 "int main () { int var; var = 4; return var * 10 + 2;}"
try 0 "int main () {return 3 + -3;}"
try 0 "int main () {return 8 - (3 + 5);}"
try 0 "int main () {return 15 + (-3*+5);}"
try 1 "int main () {return 1 <= 1;}"
try 1 "int main () {return 42 == 4 * 10 + 2;}"
try 1 "int main () {return 1 < 2;}"
try 1 "int main () {return 2 > 1;}"
try 1 "int main () {if (1) return 1;}"
try 2 "int main () {if (0) return 1; else return 2;}"
try 3 "int main () { 1; 2; return 3; }"
try 1 "int main () {if (0) { return 0; } else { return 1; }}"
try 0 "int main () { f(); return 0; }"
try 0 "int main () { return g(0); }"
try 1 "int id(int x) { return x; } int main() { return id(1); }"
try 0 "int main () { while (0) { return 1; } return 0; }"
try 1 "int main () { int x; x = 0; x = x + 1; return x; }"
try 55 "int main () { int x; int sum; x = 10; sum = 0; while (x != 0) { sum = sum + x; x = x - 1; } return sum; }"
try 0 "int main () { int x; int y; x = y = 0; return x; }"
try 55 "int main () { int i; int sum; sum = 0; for (i = 1; i <= 10; i = i + 1) { sum = sum + i; } return sum; }"
try 0 "int main () { int *x; int** y; return 0; }"
try 1 "int main () { int *x; int y; y = 1; x = &y; return *x; }"
try 1 "int main () { int *p; alloc4(&p, 1, 2, 4, 8); return *p; }"
try 4 "int main () { int *p; alloc4(&p, 1, 2, 4, 8); int *q; q = p + 2; return *q; }"
try 0 "int* id_ptr(int* x) { return x; } int main() { int x; x = 0; return *(id_ptr(&x)); }"

echo OK
