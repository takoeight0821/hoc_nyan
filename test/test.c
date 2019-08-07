extern void* stderr;

int printf();
int fprintf();
int exit();
int putchar(char c);
int puts(char* msg);
void* malloc(long size);
void* calloc(long count, long size);
#define NULL (0)

#define EXPECT(expected, expr)                                          \
  {                                                                     \
    int e1;                                                             \
    int e2;                                                             \
    e1 = (expected);                                                    \
    e2 = (expr);                                                        \
    if (e1 == e2) {                                                     \
      fprintf(stderr, "%s => %d\n", #expr, e2);                         \
    } else {                                                            \
      fprintf(stderr, "line %d: %s: %d expected, but got %d\n", __LINE__, #expr, e1, e2); \
      exit(1);                                                          \
    }                                                                   \
  } 0

int global_a;
int* global_b;
int global_c[2];

struct pair {
  int x;
  int y;
};

struct list {
  int car;
  struct list* cdr;
};

enum Enum {
  A,
  B,
  C,
};

typedef int type_a;

int id(int x) {
  return x;
}

int* id_ptr(int* x) {
  return x;
}

int func();

void void_fun() {
  return;
}

int fib(int n) {
  switch (n) {
  case 0:
  case 1:
    return 1;
  default:
    return fib(n - 1) + fib (n - 2);
  }
}

static int static_global_a;

static int static_func() {
  return 0;
}

int no_args(void) {
  return 0;
}

int main() {
  EXPECT(0, 0);
  EXPECT(42, 40 + 2);
  EXPECT(42, 2 + 4 * 10);
  EXPECT(21, 42 / 2);
  EXPECT(15, 5 * (9 - 6));
  EXPECT(4, (3 + 5) / 2);
  EXPECT(-1, 1 * -1);
  EXPECT(-1, -1 * 1);
  EXPECT(1, -1 * -1);
  EXPECT(1, -1 / -1);
  {
    int a;
    a = 42;
    EXPECT(42, a);
  }
  {
    int var;
    var = 4;
    EXPECT(42, var * 10 + 2);
  }
  EXPECT(0, 3 + -3);
  EXPECT(0, 8 - (3 + 5));
  EXPECT(0, 15 + (-3*+5));
  EXPECT(1, 1 <= 1);
  EXPECT(1, 42 == 4 * 10 + 2);
  EXPECT(1, 1 < 2);
  EXPECT(1, 2 > 1);
  {
    int tmp;
    if (1)
      tmp = 1;
    EXPECT(1, tmp);
    if (0)
      tmp = 1;
    else
      tmp = 2;
    EXPECT(2, tmp);
    tmp = 0;
    while (0) {
      tmp = 1;
    }
    EXPECT(0, tmp);
  }
  EXPECT(1, id(1));
  EXPECT(1, id(id(1)));
  EXPECT(2, id(id(1)) + 1);
  {
    int x;
    x = 0;
    x = x + 1;
    EXPECT(1, x);
  }
  {
    int x;
    x = 10;
    int sum;
    sum = 0;
    while (x != 0) {
      sum = sum + x;
      x = x - 1;
    }
    EXPECT(55, sum);
  }
  {
    int x;
    int y;
    x = y = 0;
    EXPECT(0, x);
  }
  {
    int i;
    int sum;
    sum = 0;
    for (i = 1; i <= 10; i = i + 1) {
      sum = sum + i;
    }
    EXPECT(55, sum);
  }
  {
    int *x;
    int y;
    y = 1;
    x = &y;
    EXPECT(1, *x);
  }
  {
    int *x;
    int y;
    y = 1;
    x = &y;
    *x = 2;
    EXPECT(2, *x);
  }
  {
    int x;
    x = 0;
    EXPECT(0, *(id_ptr(&x)));
  }
  EXPECT(10, func());
  EXPECT(4, sizeof(1));
  EXPECT(4, sizeof(int));
  {
    int arr[3];
    *arr = 1;
    EXPECT(1, *arr);
  }
  {
    int arr[3];
    int *p;
    p = arr + 1;
    *p = 4;
    EXPECT(4, *p);
  }
  {
    int arr[3];
    *(arr + 1) = 4;
    EXPECT(4, *(arr + 1));
  }
  {
    char c;
    c = 65;
    EXPECT(65, c);
  }
  {
    char c;
    c = 65;
    putchar(c);
  }
  {
    char msg[3];
    *msg = 104;
    *(msg + 1) = 105;
    *(msg + 2) = 0;
    puts(msg);
  }
  {
    int arr[3];
    arr[1] = 4;
    EXPECT(4, arr[1]);
  }
  {
    char msg[3];
    msg[0] = 104;
    msg[1] = 105;
    msg[2] = 0;
    puts(msg);
  }
  {
    char *msg;
    msg = "hello, world";
    puts(msg);
  }
  puts("hello, ");
  puts("world");
  global_a = 1;
  EXPECT(1, global_a);
  {
    int b;
    b = 2;
    global_b = &b;
    EXPECT(2, *global_b);
  }
  global_c[0] = 1;
  EXPECT(1, global_c[0]);
  {
    int a;
    a = 1;
    {
      int a;
      a = 2;
      EXPECT(2, a);
    }
  }
  {
    int a;
    a = 1;
    {
      int a;
      a = 2;
    }
    EXPECT(1, a);
  }
  {
    struct pair p;
    p.x = 0;
    p.y = 1;
    EXPECT(0, p.x);
  }
  {
    struct pair p;
    p.x = 0;
    p.y = 1;
    EXPECT(1, p.y);
  }
  {
    struct pair p;
    p.x = 1;
    p.y = 2;
    EXPECT(3, p.x + p.y);
  }
  void_fun();
  {
    long a;
    a = 1;
    EXPECT(1, a);
  }
  {
    long a;
    int b;
    a = 1;
    b = a;
    EXPECT(1, b);
  }
  {
    type_a a;
    a = 0;
    EXPECT(0, a);
  }
  EXPECT(0, A);
  EXPECT(1, B);
  EXPECT(2, C);
  EXPECT(0, !1);
  EXPECT(1, !0);
  EXPECT(0, !(1 == 1));
  EXPECT(1, !(1 != 1));
  EXPECT(1, 1 && 1);
  EXPECT(1, 0 || 1);
  EXPECT(0, 1 && 0 || 0);
  EXPECT(1, 7%2);
  EXPECT(65, 'A');
  {
    struct pair* a;
    a = malloc(sizeof(struct pair));
    (*a).x = 4;
    (*a).y = 2;
    EXPECT(42, (*a).x * 10 + (*a).y);
  }
  {
    struct pair* a;
    a = malloc(sizeof(struct pair));
    a->x = 4;
    a->y = 2;
    EXPECT(42, a->x * 10 + a->y);
  }
  {
    int *a;
    a = calloc(10, sizeof(int));
    int *b[3];
    b[0] = a;
    b[1] = a;
    b[2] = a;
    a[4] = 42;
    EXPECT(42, b[0][4]);
  }
  {
    int a = 42;
    EXPECT(42, a);
  }
  {
    int sum = 0;
    for (int a = 0; a < 10; a = a + 1) {
      sum = sum + a;
    }
    EXPECT(45, sum);
  }
  EXPECT(0, static_func());
  EXPECT(10, '\n');
  EXPECT(9, '\t');
  EXPECT(1, printf("\""));
  switch (0) {
  case 0:
    printf("case 0\n");
  case 1:
    printf("case 0 or 1\n");
    break;
  case 2:
    printf("case 2\n");
    break;
  }
  EXPECT(21, fib(7));
  EXPECT(2, (1, 2));
  {
    int a = 0;
    EXPECT(1, a = a + 1);
    EXPECT(2, a += 1);
    EXPECT(2, a++);
    EXPECT(3, a);
  }
  {
    struct list* nil = NULL;
    struct list a;
    a.car = 1;
    a.cdr = nil;
    EXPECT(1, a.car);
  }
  {
    int a[2][3];
    EXPECT(sizeof(a), 24);
    EXPECT(sizeof(a[0]), 12);
    EXPECT(sizeof(a[0][0]), 4);
  }
  {
    int i = 1;
    int sum = 0;
    for (; i < 10;) {
      sum += i++;
    }
    EXPECT(sum, 45);
  }
  return 0;
}

int func() {
  return 10;
}
