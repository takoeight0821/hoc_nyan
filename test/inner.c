#include "../src/hoc.h"
typedef struct lock {
  int locked;
} Lock;

typedef struct elem {
  int type;
  Lock lock;
} Elem;

void lock(Elem* elem) {
  elem->locked = 1;
}

int main() {
  Elem* elem = calloc(1, sizeof(Elem));
  lock(elem);
  printf("%d\n", elem->lock.locked);
}
