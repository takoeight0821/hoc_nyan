#define A return 0
#define F(P) P
#define G(A, B) (A + B)

int main(void) {
  A;
  F(return 2);
  return G(2, 3);
  return 1;
}
