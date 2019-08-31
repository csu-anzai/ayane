#include "main.h"

void *xmalloc(size_t bytes) {
  auto r = malloc(bytes);
  if (!r) {
    perror("malloc");
    exit(1);
  }
#ifdef DEBUG
  memset(r, 0xcc, bytes);
#endif
  return r;
}

void *xcalloc(size_t n, size_t size) {
  auto r = calloc(n, size);
  if (!r) {
    perror("calloc");
    exit(1);
  }
  return r;
}

void *xrealloc(void *p, size_t bytes) {
  auto r = realloc(p, bytes);
  if (!r) {
    perror("realloc");
    exit(1);
  }
  return r;
}

void *mmalloc(int bytes) {
  bytes = bytes + 7 & ~7;
  static char *end;
  static char *p;
  if (end - p < bytes) {
    auto chunk = std::max(bytes, 10000);
    p = (char *)xmalloc(chunk);
    end = p + chunk;
  }
  auto r = p;
#ifdef DEBUG
  memset(r, 0xcc, bytes);
#endif
  p += bytes;
  return r;
}
