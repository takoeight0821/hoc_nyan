#!/bin/bash

try() {
    expected="$1"
    input="$2"

    echo "$input" > tmp_c
    ./hoc tmp_c > tmp.s
    gcc -c test.c -o test.o
    gcc -o tmp.out tmp.s test.o
    ./tmp.out
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
        rm tmp_c tmp.s tmp.out
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
try 1 "int id(int x) { return x; } int main() { return id(1); }"
try 1 "int id(int x) { return x; } int main() { return id(id(1)); }"
try 2 "int id(int x) { return x; } int main() { return id(id(1) + 1); }"
try 0 "int main () { while (0) { return 1; } return 0; }"
try 1 "int main () { int x; x = 0; x = x + 1; return x; }"
try 55 "int main () { int x; int sum; x = 10; sum = 0; while (x != 0) { sum = sum + x; x = x - 1; } return sum; }"
try 0 "int main () { int x; int y; x = y = 0; return x; }"
try 55 "int main () { int i; int sum; sum = 0; for (i = 1; i <= 10; i = i + 1) { sum = sum + i; } return sum; }"
try 0 "int main () { int *x; int** y; return 0; }"
try 1 "int main () { int *x; int y; y = 1; x = &y; return *x; }"
try 2 "int main () { int *x; int y; y = 1; x = &y; *x = 2; return *x; }"
try 1 "int main () { int *p; alloc4(&p, 1, 2, 4, 8); return *p; }"
try 4 "int main () { int *p; alloc4(&p, 1, 2, 4, 8); int *q; q = p + 2; return *q; }"
try 2 "int main () { int *p; alloc4(&p, 1, 2, 4, 8); int *q; q = p + 2; q = q - 1; return *q; }"
try 0 "int* id_ptr(int* x) { return x; } int main() { int x; x = 0; return *(id_ptr(&x)); }"
try 10 "int func(); int main() { return func(); } int func() { return 10; }"
try 4 "int main() { return sizeof(1); }"
try 1 "int main() { int arr[3]; *arr = 1; return *arr; }"
try 4 "int main() { int arr[3]; int *p; p = arr + 1; *p = 4; return *p; }"
try 4 "int main() { int arr[3]; *(arr + 1) = 4; return *(arr + 1); }"
try 65 "int main() { char c; c = 65; return c; }"
try 65 "int main() { char c; c = 65; putchar(c); return c; }"
try 0 "int main() { char msg[3]; *msg = 104; *(msg + 1) = 105; *(msg + 2) = 0; puts(msg); return 0; }"
try 4 "int main() { int arr[3]; arr[1] = 4; return arr[1]; }"
try 0 "int main() { char msg[3]; msg[0] = 104; msg[1] = 105; msg[2] = 0; puts(msg); return 0; }"
try 0 "int main() { char *msg; msg = \"hello, world\"; puts(msg); return 0; }"
try 0 "int main() { puts(\"hello, \"); puts(\"world\"); return 0; }"
try 1 "int a; int main() { a = 1; return a; }"
try 2 "int* a; int main() { int b; b = 2; a = &b; return *a; }"
try 1 "int a[2]; int main() { a[0] = 1; return a[0]; }"
try 2 "int main () { int a; a = 1; { int a; a = 2; return a; }}"
try 1 "int main () { int a; a = 1; { int a; a = 2; } return a; }"
try 0 "struct pair { int x; int y; }; int main() { struct pair p; return 0; }"
try 0 "struct pair { int x; int y; }; int main() { struct pair p; p.x = 0; p.y = 1; return p.x; }"
try 1 "struct pair { int x; int y; }; int main() { struct pair p; p.x = 0; p.y = 1; return p.x + p.y; }"
try 2 "struct pair { int x; int y; }; int main() { struct pair p; p.x = 1; p.y = 2; return p.y; }"
try 0 "void f() { } int main() { f(); return 0; }"
try 0 "struct pair { char* x; char* y; }; void f(struct pair* p) { puts((*p).x); } int main() { struct pair p; p.x = \"x\"; p.y = \"y\"; f(&p); return 0; }"

echo OK
