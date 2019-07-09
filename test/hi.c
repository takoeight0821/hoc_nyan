int puts(char* s);

int main() {
  char msg[3];
  *msg = 104;
  *(msg + 1) = 105;
  *(msg + 2) = 0;
  puts(msg);
  return 0;
}
