inline void fpr(FILE *F, char c) { fputc(c, F); }
inline void fpr(FILE *F, const char *s) { fputs(s, F); }
inline void fpr(FILE *F, double a) { fprintf(F, "%f", a); }
inline void fpr(FILE *F, int a) { fprintf(F, "%d", a); }
inline void fpr(FILE *F, size_t a) { fprintf(F, "%zu", a); }
inline void fpr(FILE *F, void *p) { fprintf(F, "%p", p); }

#ifdef DEBUG
void stacktrace();
bool assertfail(const char *filename, int line, const char *s);
#define assert(a) (a) || assertfail(__FILE__, __LINE__, #a)
#define debug(a)                                                               \
  do {                                                                         \
    fprintf(stderr, "%s:%d: %s: ", __FILE__, __LINE__, #a);                    \
    fpr(stderr, a);                                                            \
    fputc('\n', stderr);                                                       \
  } while (0)
#else
#define stacktrace()
#define assert(a)
#define debug(a)
#endif

unsigned fnv(const void *p, int bytes);
