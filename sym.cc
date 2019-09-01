#include "main.h"

namespace {
struct key {
  const char *s;
  int n;

  key(const char *s, int n) : s(s), n(n) {}
  key(sym *t) : s(t->s), n(t->n) {}

  unsigned hash() const { return fnv(s, n); }

  bool operator==(sym *t) const {
    if (n != t->n)
      return false;
    return !memcmp(s, t->s, n);
  }

  sym *store() const {
    auto r = (sym *)mmalloc(offsetof(sym, s) + n);
    r->val = 0;
    r->n = n;
    memcpy(r->s, s, n);
    return r;
  }
};

set<key, sym> syms;

struct init {
  init() {
    for (int i = 0; i != nkeywords; ++i) {
      auto t = keywords + i;
      assert(t->n < sizeof t->s);
      key k(t);
      syms.add(k, t);
    }
  }
} init1;
} // namespace

sym *intern(const char *s, int n) {
  if (n > 0xffff)
    err("Symbol too long");
  key k(s, n);
  return syms[k];
}
