int fib_0(int n) {
  if (n == 0) {
    return 1;
  } else if (n == 1) {
    return 1;
  } else {
    return fib_0(n - 1) + fib_0(n - 2);
  }
}

int fib_1(int n) {
  switch (n) {
  case 0:
  case 1:
    return 1;
  default:
    return fib_1(n - 1) + fib_1(n - 2);
  }
}
int main() {
  return fib_0(10) == fib_1(10);
}
