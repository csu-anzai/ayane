#include "main.h"
#include <fcntl.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#define O_BINARY 0
#endif

// current file
const char *filename;
// beginning of source text
const char *filesrc;
// current position in source text
const char *src;

// current token
const char *toksrc;
int tok;
sym *toksym;
vec<char> buf;

srcfile::srcfile(const char *filename)
    : old_filename(::filename), old_filesrc(::filesrc), old_src(::src) {
  char *s = 0;
  size_t n = 0;
  if (strcmp(filename, "stdin")) {
    auto f = open(filename, O_BINARY | O_RDONLY);
    struct stat st;
    if (f < 0 || fstat(f, &st)) {
      perror(filename);
      exit(1);
    }

    n = st.st_size;
    s = (char *)xmalloc(n + 2);
    if (read(f, s, n) != n) {
      perror("read");
      exit(1);
    }

    close(f);
  } else {
#ifdef _WIN32
    _setmode(0, O_BINARY);
#endif
    int chunk = 1 << 20;
    size_t cap = 0;
    for (;;) {
      if (n + chunk + 2 > cap) {
        cap = std::max(n + chunk + 2, cap * 2);
        s = (char *)xrealloc(s, cap);
      }

      auto r = read(0, s + n, chunk);
      if (r < 0) {
        perror("read");
        exit(1);
      }
      n += r;
      if (r != chunk)
        break;
    }
  }
  s[n] = 0;
  if (n && s[n - 1] != '\n') {
    s[n] = '\n';
    s[n + 1] = 0;
  }
  ::filename = filename;
  filesrc = src = s;
  toksrc = 0;
}

srcfile::~srcfile() {
  free((void *)filesrc);

  ::filename = old_filename;
  ::filesrc = old_filesrc;
  ::src = old_src;
}

#ifdef _MSC_VER
__declspec(noreturn)
#endif
    void err(const char *msg) {
  if (filename && filesrc && toksrc) {
    // line number
    int line = 1;
    for (auto s = filesrc; s != toksrc; ++s)
      if (*s == '\n')
        ++line;

    // beginning of line
    auto s0 = toksrc;
    while (!(s0 == filesrc || s0[-1] == '\n'))
      --s0;

    // print context
    for (auto s1 = s0; *s1 >= ' '; ++s1)
      fputc(*s1, stderr);
    fputc('\n', stderr);
    for (auto s1 = s0; s1 != toksrc; ++s1)
      fputc(*s1 == '\t' ? '\t' : ' ', stderr);
    fprintf(stderr, "^\n");
    fprintf(stderr, "%s:%d: ", filename, line);
  }
  fprintf(stderr, "%s\n", msg);
  exit(1);
}
