#include "main.h"

namespace {
char *mem_begin;
char *mem_end;
char *mem;
} // namespace

void initmem(double memlimit) {
  auto n = (size_t)(memlimit * (1 << 30));
  mem_begin = mem = (char *)xmalloc(n);
  mem_end = mem + n;
}

void *alloc(size_t bytes) {
  assert(!(bytes & sizeof(void *) - 1));
  if (mem_end - mem < bytes)
    err("out of memory");
  auto r = mem;
  mem += bytes;
#ifdef DEBUG
  memset(r, 0xcc, bytes);
#endif
  return r;
}

void clearmem() { mem = mem_begin; }

// terms

type *term::ty() {
  switch (tag) {
  case t_add:
  case t_call:
  case t_mul:
  case t_sub:
    return args[0]->ty();
  case t_all:
  case t_and:
  case t_eq:
  case t_eqv:
  case t_exists:
  case t_false:
  case t_not:
  case t_or:
  case t_true:
    return &Bool;
  case t_distinct_object:
    return &Ind;
  case t_id:
  case t_var:
    return ty_;
  case t_int:
    return &Int;
  case t_rat:
    return &Rat;
  case t_real:
    return &Real;
  }
  assert(false);
  return 0;
}

// globals

term False = {t_false};
term True = {t_true};

// construct

term *atom(tag_t tag, int bytes) {
  auto r = (term *)alloc(offsetof(term, args) + bytes);
  r->tag = tag;
  r->n = 0;
  return r;
}

term *constant(type *ty, sym *name) {
  auto r = atom(t_id, sizeof(type *) + sizeof(sym *));
  r->ty_ = ty;
  r->name = name;
  return r;
}

term *var(type *ty) {
  auto r = atom(t_var, sizeof(type *));
  r->ty_ = ty;
  return r;
}

static term *compound(tag_t tag, int n) {
  auto r = (term *)alloc(offsetof(term, args) + n * sizeof(term *));
  r->tag = tag;
  r->n = n;
  return r;
}

term *make(tag_t tag, term *a) {
  auto r = compound(tag, 1);
  r->args[0] = a;
  return r;
}

term *make(tag_t tag, term *a, term *b) {
  auto r = compound(tag, 2);
  r->args[0] = a;
  r->args[1] = b;
  return r;
}

term *make(tag_t tag, term *a, term *b, term *c) {
  auto r = compound(tag, 3);
  r->args[0] = a;
  r->args[1] = b;
  r->args[2] = c;
  return r;
}

term *make(tag_t tag, term *a, const vec<term *> &v) {
  auto r = compound(tag, 1 + v.n);
  r->args[0] = a;
  memcpy(r->args + 1, v.p, v.n * sizeof a);
  return r;
}

term *make(tag_t tag, const vec<term *> &v) {
  auto r = compound(tag, v.n);
  memcpy(r->args, v.p, v.n * sizeof(term *));
  return r;
}

term *implies(term *a, term *b) { return make(t_or, make(t_not, a), b); }

namespace {
vec<term *> bound_vars;

void get_free_vars1(term *a) {
  switch (a->tag) {
  case t_all:
  case t_exists: {
    auto old_size = bound_vars.n;
    for (int i = 1; i != a->n; ++i)
      bound_vars.push(at(a, i));
    get_free_vars1(at(a, 0));
    bound_vars.resize(old_size);
    return;
  }
  case t_var:
    if (std::find(bound_vars.begin(), bound_vars.end(), a) != bound_vars.end())
      return;
    if (std::find(free_vars.begin(), free_vars.end(), a) != free_vars.end())
      return;
    free_vars.push(a);
    return;
  }
  for (auto i : a)
    get_free_vars1(at(a, i));
}
} // namespace

vec<term *> free_vars;

void get_free_vars(term *a) {
  assert(!bound_vars.n);
  free_vars.n = 0;
  get_free_vars1(a);
}

bool eq(term *a, term *b) {
  assert(a->ty() == b->ty());

  // same term
  if (a == b)
    return true;

  // different tags
  if (a->tag != b->tag)
    return false;

  // atoms
  switch (a->tag) {
  case t_int:
    return mpz_cmp(a->int_val, b->int_val) == 0;
  case t_rat:
  case t_real:
    return mpq_equal(a->rat_val, b->rat_val);
  }
  if (!a->n)
    return 0;

  // compound terms
  if (a->n != b->n)
    return false;
  for (auto i : a)
    if (!eq(at(a, i), at(b, i)))
      return false;
  return true;
}

// clauses

bool complete;

// construct

vec<term *> neg, pos;

static int depth(term *a) {
  if (!a->n)
    return 0;
  int r = 0;
  for (auto i : a)
    r = std::max(r, depth(at(a, i)));
  return r + 1;
}

void make_clause() {}

// etc

namespace {
vec<std::pair<term *, term *>> vars;

term *fresh_vars(term *a) {
  if (a->tag == t_var) {
    for (auto p : vars)
      if (p.first == a)
        return p.second;
    auto p = std::make_pair(a, var(a->ty_));
    vars.push(p);
    return p.second;
  }

  if (a->n == 0)
    return a;

  auto b = compound(a->tag, a->n);
  for (auto i : a)
    b->args[i] = fresh_vars(at(a, i));
  return b;
}
} // namespace

clause *fresh_vars(clause *c) {
  auto d = (clause *)alloc(offsetof(clause, literals) + c->n * sizeof(term *));
  d->dead = false;
  d->neg_n = c->neg_n;
  d->n = c->n;
  vars.n = 0;
  for (auto i : c)
    d->literals[i] = fresh_vars(at(c, i));
  return d;
}
