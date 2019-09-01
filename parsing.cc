#include "main.h"
#include <fcntl.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#define O_BINARY 0
#endif

// SZS status codes
const char *szs[] = {
#define _(s) #s,
#include "szs.h"
#undef _
};

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
term *tokterm;
vec<char> buf;

// source files

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

// metadata
bool conjecture;
int status;

// arbitrary-precision numbers

namespace {
bool sign() {
  switch (*src) {
  case '+':
    ++src;
    break;
  case '-':
    buf.push(*src++);
    return true;
  }
  return false;
}

void digits() {
  auto s = src;
  if (!('0' <= *s && *s <= '9')) {
    toksrc = s;
    err("Expected digit");
  }
  do
    buf.push(*s++);
  while ('0' <= *s && *s <= '9');
  src = s;
}
} // namespace

void parsenum() {
  buf.n = 0;
  auto sgn = sign();
  auto s = src;
  digits();
  switch (*src) {
  case '.':
  case 'E':
  case 'e': {
    // integer part
    buf.push(0);
    mpz_t integer;
    mpz_init_set_str(integer, buf.p + sgn, 10);

    // decimal part
    mpz_t decimal;
    mpz_init(decimal);
    unsigned scale = 0;
    if (*src == '.') {
      buf.n = 0;
      src++;
      digits();
      buf.push(0);
      mpz_set_str(decimal, buf.p, 10);
      scale = buf.n - 1;
    }
    mpz_t pow_scale;
    mpz_init(pow_scale);
    mpz_ui_pow_ui(pow_scale, 10, scale);

    // mantissa
    mpz_t mantissa;
    mpz_init_set(mantissa, decimal);
    mpz_addmul(mantissa, integer, pow_scale);
    if (sgn)
      mpz_neg(mantissa, mantissa);

    // exponent
    bool exponent_sign = false;
    unsigned long exponent = 0;
    if (*src == 'e' || *src == 'E') {
      auto s = ++src;
      exponent_sign = sign();
      errno = 0;
      exponent = strtoul(src, (char **)&src, 10);
      if (errno) {
        toksrc = s;
        err(strerror(errno));
      }
    }
    mpz_t pow_exponent;
    mpz_init(pow_exponent);
    mpz_ui_pow_ui(pow_exponent, 10, exponent);

    // result
    tokterm = atom(Real, sizeof(mpq_t));
    mpq_init(tokterm->rat_val);
    mpq_set_num(tokterm->rat_val, mantissa);
    mpq_set_den(tokterm->rat_val, pow_scale);
    if (!exponent_sign)
      mpz_mul(mpq_numref(tokterm->rat_val), mpq_numref(tokterm->rat_val),
              pow_exponent);
    else
      mpz_mul(mpq_denref(tokterm->rat_val), mpq_denref(tokterm->rat_val),
              pow_exponent);
    mpq_canonicalize(tokterm->rat_val);

    // cleanup
    mpz_clear(integer);
    mpz_clear(decimal);
    mpz_clear(pow_scale);
    mpz_clear(mantissa);
    mpz_clear(pow_exponent);
    break;
  }
  case '/':
    buf.push(*src++);
    digits();
    buf.push(0);
    tokterm = atom(Rat, sizeof(mpq_t));
    mpq_init(tokterm->rat_val);
    if (mpq_set_str(tokterm->rat_val, buf.p, 10))
      err("invalid number");
    mpq_canonicalize(tokterm->rat_val);
    break;
  default:
    buf.push(0);
    tokterm = atom(Int, sizeof(mpz_t));
    mpz_init_set_str(tokterm->int_val, buf.p, 10);
    break;
  }
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
