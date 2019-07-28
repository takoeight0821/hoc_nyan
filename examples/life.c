/* #include <stdio.h> */
/* #include <unistd.h> */
/* #include <stdlib.h> */
/* #include <time.h> */
/* #include <stdbool.h> */

typedef int bool;
#define NULL 0
extern void* stderr;

void* malloc();
void* calloc();
int rand();
void free();
int printf();
void srand();
long time();
int fprintf();
void exit();
int sscanf();
void sleep();

struct field {
  int x_width;
  int y_width;
  bool** cells;
};

bool is_valid(struct field* f, int x, int y) {
  return !(x >= f->x_width || y >= f->y_width);
}

bool view(struct field* f, int x, int y) {
  return f->cells[x+1][y+1];
}

void set(struct field* f, int x, int y, bool s) {
  f->cells[x+1][y+1] = s;
}

struct field* make_field(int x_width, int y_width, bool** pattern) {
  struct field* f = malloc(sizeof(struct field));

  bool** cells = calloc(sizeof(bool*), x_width + 2);
  for (int x = 0; x < x_width + 2; x = x + 1) {
    cells[x] = calloc(sizeof(bool), y_width + 2);
  }

  if (pattern == NULL) {
    for (int x = 1; x < x_width + 1; x = x + 1) {
      for (int y = 1; y < y_width + 1; y = y + 1) {
        cells[x][y] = rand() % 2;
      }
    }
  } else {
    for (int x = 1; x < x_width + 1; x = x + 1) {
      for (int y = 1; y < y_width + 1; y = y + 1) {
        cells[x][y] = pattern[x][y];
      }
    }
  }

  f->x_width = x_width;
  f->y_width = y_width;
  f->cells = cells;

  return f;
}

void free_field(struct field* f) {
  for (int x = 0; x < f->x_width + 2; x = x + 1) {
    free(f->cells[x]);
  }
  free(f->cells);
  free(f);
}

char to_char(bool c) {
  if (c) {
    return '#';
  } else {
    return '_';
  }
}

void print_field(struct field* f) {
  for (int x = 0; x < f->x_width; x = x + 1) {
    for (int y = 0; y < f->y_width; y = y + 1) {
      printf("%c", to_char(view(f, x, y)));
    }
    printf("\n");
  }
}

bool next_state(struct field* f, int x, int y) {
  int living = 0;
  living = living + view(f, x-1, y-1) + view(f, x-1, y) + view(f, x-1, y+1);
  living = living + view(f, x, y-1) + view(f, x, y+1);
  living = living + view(f, x+1, y-1) + view(f, x+1, y) + view(f, x+1, y+1);

  if (view(f, x, y)) {
    return living == 2 || living == 3;
  } else {
    return living == 3;
  }
}

void update_field(struct field* f) {
  struct field* copy = make_field(f->x_width, f->y_width, f->cells);


  for (int x = 0; x < f->x_width; x = x + 1) {
    for (int y = 0; y < f->y_width; y = y + 1) {
      int s = next_state(copy, x, y);
      set(f, x, y, s);
    }
  }
}

int main(int argc, char** argv) {
  srand(time(NULL));

  if (argc != 4) {
    fprintf(stderr, "usage: %s x y times\n", argv[0]);
    exit(1);
  }

  int x;
  int y;
  int times;
  sscanf(argv[1], "%d", &x);
  sscanf(argv[2], "%d", &y);
  sscanf(argv[3], "%d", &times);

  struct field* f = make_field(x, y, NULL);
  print_field(f);

  for (int i = 0; i < times; i = i + 1) {
    for (int j = 0; j < y; j = j + 1) {
      printf("-");
    }
    printf("\n");
    sleep(1);
    update_field(f);
    print_field(f);
  }

  free_field(f);
  return 0;
}
