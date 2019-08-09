int print_int(int i);
int puts(char* s);

int sort(int* arr, int n) {
  int tmp = 0;
  for (int i = 0; i < n; i = i + 1) {
    for (int j = 0; j < n - 1; j = j + 1) {
      if (arr[j + 1] < arr[j]) {
        tmp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = tmp;
      }
    }
  }

  return 0;
}

int main() {
  int array[10];
  array[0] = 32;
  array[1] = 41;
  array[2] = 10;
  array[3] = -1;
  array[4] = 0;
  array[5] = 100;
  array[6] = 8;
  array[7] = 184;
  array[8] = 2;
  array[9] = -12;

  for (int i = 0; i < 10; i = i + 1) {
    print_int(array[i]);
    puts(";");
  }

  sort(array, 10);

  for (int i = 0; i < 10; i = i + 1) {
    print_int(array[i]);
    puts(";");
  }

  return 0;
}
