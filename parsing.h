// SZS status codes
enum {
#define _(s) s,
#include "szs.h"
#undef _
  n_szs
};

extern const char *szs[];

// current file
extern const char *filename;
// beginning of source text
extern const char *filesrc;
// current position in source text
extern const char *src;

// current token
extern const char *toksrc;
extern int tok;
extern sym *toksym;
extern term *tokterm;
extern vec<char> buf;

// source files
struct srcfile {
  // remember previous file
  // for nested includes
  // and so that after parsing is done
  // error reporting will know we are no longer in a file
  const char *old_filename;
  const char *old_filesrc;
  const char *old_src;

  srcfile(const char *filename);
  ~srcfile();
};

// metadata
extern bool conjecture;
extern int status;

// arbitrary-precision numbers
void parse_number();

// error
#ifdef _MSC_VER
__declspec(noreturn)
#endif
    void err(const char *s);
