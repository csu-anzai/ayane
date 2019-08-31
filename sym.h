struct sym {
  unsigned short n;
  char s[0x20 - sizeof(unsigned short)];
};

extern sym keywords[];

inline size_t keyword(sym *s) {
  size_t i = (char *)s - (char *)keywords;
  return i / sizeof(sym);
}

sym *intern(const char *s, int n);

inline void fpr(FILE *F, sym *s) { fwrite(s->s, 1, s->n, F); }
