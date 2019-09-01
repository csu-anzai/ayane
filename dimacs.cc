#include "main.h"

namespace {
enum {
  k_eof,
  k_num,
  k_zero,
};

void lex() {
loop:
  auto s = src;
  toksrc = s;
  switch (*src) {
  case ' ':
  case '\f':
  case '\n':
  case '\r':
  case '\t':
  case '\v':
    src = s + 1;
    goto loop;
  case '0':
    if (!isdigit(s[1])) {
      src = s + 1;
      tok = k_zero;
      return;
    }
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    do
      ++s;
    while (isdigit(*s));
    toksym = intern(src, s - src);
    src = s;
    tok = k_num;
    return;
  case 'c':
#ifdef DEBUG
    if (!status)
      for (; *s != '\n'; ++s) {
        if (!memcmp(s, "SAT", 3)) {
          status = Satisfiable;
          break;
        }
        if (!memcmp(s, "UNSAT", 5)) {
          status = Unsatisfiable;
          break;
        }
      }
#endif
    src = strchr(s, '\n');
    goto loop;
  case 0:
    tok = k_eof;
    return;
  }
  src = s + 1;
  tok = *s;
}

term *num() {
  auto a = constant(&Bool, toksym);
  lex();
  return a;
}
} // namespace

void read_dimacs(const char *filename) {
  srcfile file(filename);
  lex();
  if (tok == 'p') {
    while (isspace(*src))
      ++src;

    if (!(src[0] == 'c' && src[1] == 'n' && src[2] == 'f'))
      err("Expected 'cnf'");
    src += 3;
    lex();

    if (tok != k_num)
      err("Expected count");
    lex();

    if (tok != k_num)
      err("Expected count");
    lex();
  }
  for (;;)
    switch (tok) {
    case '-':
      neg.push(num());
      break;
    case k_eof:
      if (neg.n | pos.n)
        make_clause();
      return;
    case k_num:
      pos.push(num());
      break;
    case k_zero:
      make_clause();
      lex();
      break;
    default:
      err("Syntax error");
    }
}
