#include "main.h"

namespace {
term *skolem(type *ty, const vec<term *> &v) {
  auto a = constant(ty, 0);
  if (!v.n)
    return a;
  // don't care about params because type check is already done
  return mk(Call, a, v);
}

term *skolem(type *ty, const vec<std::pair<term *, term *>> &v) {
  vec<term *> w(v.n);
  for (int i = 0; i != v.n; ++i)
    w[i] = v[i].second;
  return skolem(ty, w);
}

// rename a term to avoid exponential blowup

bool is_complex(term *a) {
  while (a->tag == Not)
    a = at(a, 0);
  switch (a->tag) {
  case All:
  case Eqv:
  case Exists:
    assert(false);
  case And:
  case Or:
    return true;
  }
  return false;
}

term *ren_eqv(term *a) {
  if (!is_complex(a))
    return a;
  get_free_vars(a);
  auto b = skolem(0, free_vars);
  cnf(mk(And, implies(b, a), implies(a, b)));
  return b;
}

term *ren_pos(term *a) {
  get_free_vars(a);
  auto b = skolem(0, free_vars);
  cnf(implies(b, a));
  return b;
}

// negation normal form
// for-all vars map to fresh vars
// exists vars map to skolem functions

term *nnf(vec<std::pair<term *, term *>> &all_vars,
          vec<std::pair<term *, term *>> &exists_vars, bool pol, term *a);

term *nnf_args(vec<std::pair<term *, term *>> &all_vars,
               vec<std::pair<term *, term *>> &exists_vars, bool pol, term *a,
               tag_t tag) {
  vec<term *> v(a->n);
  for (auto i : a)
    v[i] = nnf(all_vars, exists_vars, pol, at(a, i));
  return mk(tag, v);
}

term *nnf_literal(bool pol, term *a) {
  switch (a->tag) {
  case All:
  case And:
  case Eqv:
  case Exists:
  case Or:
    assert(false);
  case False:
    return mk(!pol);
  case Not:
    return nnf_literal(!pol, at(a, 0));
  case True:
    return mk(pol);
  }
  return pol ? a : mk(Not, a);
}

term *nnf_all(vec<std::pair<term *, term *>> &all_vars,
              vec<std::pair<term *, term *>> &exists_vars, bool pol, term *a) {
  auto old_size = all_vars.n;
  all_vars.resize(old_size + a->n - 1);
  for (int i = 1; i != a->n; ++i)
    all_vars[old_size + i - 1] = std::make_pair(at(a, i), var(at(a, i)->ty()));
  a = nnf(all_vars, exists_vars, pol, at(a, 0));
  all_vars.n = old_size;
  return a;
}

term *nnf_exists(vec<std::pair<term *, term *>> &all_vars,
                 vec<std::pair<term *, term *>> &exists_vars, bool pol,
                 term *a) {
  auto old_size = exists_vars.n;
  exists_vars.resize(old_size + a->n - 1);
  for (int i = 1; i != a->n; ++i)
    exists_vars[old_size + i - 1] =
        std::make_pair(at(a, i), skolem(at(a, i)->ty(), all_vars));
  a = nnf(all_vars, exists_vars, pol, at(a, 0));
  exists_vars.n = old_size;
  return a;
}

term *nnf(vec<std::pair<term *, term *>> &all_vars,
          vec<std::pair<term *, term *>> &exists_vars, bool pol, term *a) {
  switch (a->tag) {
  case And:
    return nnf_args(all_vars, exists_vars, pol, a, pol ? And : Or);
  case Eqv: {
    auto x = ren_eqv(nnf(all_vars, exists_vars, true, at(a, 0)));
    auto y = ren_eqv(nnf(all_vars, exists_vars, true, at(a, 1)));
    return mk(And,
              // x => y
              mk(Or, nnf_literal(false, x), nnf_literal(pol, y)),
              // x <= y
              mk(Or, nnf_literal(true, x), nnf_literal(!pol, y)));
  }
  case All:
    if (pol)
      return nnf_all(all_vars, exists_vars, pol, a);
    else
      return nnf_exists(all_vars, exists_vars, pol, a);
  case Exists:
    if (pol)
      return nnf_exists(all_vars, exists_vars, pol, a);
    else
      return nnf_all(all_vars, exists_vars, pol, a);
  case False:
    return mk(!pol);
  case Or:
    return nnf_args(all_vars, exists_vars, pol, a, pol ? Or : And);
  case True:
    return mk(pol);
  case Var:
    for (auto p : all_vars)
      if (p.first == a)
        return p.second;
    for (auto p : exists_vars)
      if (p.first == a)
        return p.second;
    assert(false);
  case Not:
    return nnf(all_vars, exists_vars, !pol, at(a, 0));
  }
  if (a->n)
    a = nnf_args(all_vars, exists_vars, true, a, a->tag);
  return pol ? a : mk(Not, a);
}

// distribute OR into AND
// return:
// at most one layer of AND
// any number of layers of OR

int and_size(term *a) {
  if (a->tag == And)
    return a->n;
  return 1;
}

term *and_at(term *a, int i) {
  if (a->tag == And)
    return at(a, i);
  assert(!i);
  return a;
}

term *distribute(term *a) {
  switch (a->tag) {
  case And: {
    vec<term *> v;
    for (auto i : a) {
      auto b = distribute(at(a, i));
      if (b->tag == And) {
        v.insert(v.end(), b->args, b->args + b->n);
        continue;
      }
      v.push(b);
    }
    return mk(And, v);
  }
  case Or: {
    // flat layer of ANDs
    int64_t n = 1;
    vec<term *> ands(a->n);
    for (auto i : a) {
      auto b = distribute(at(a, i));
      if (n > 1 && and_size(b) > 1 && n * and_size(b) > 4)
        b = ren_pos(b);
      n *= and_size(b);
      ands[i] = b;
    }

    // cartesian product of ANDs
    vec<int> j(ands.n);
    memset(j.p, 0, j.n * sizeof(int));
    vec<term *> or_args(ands.n);
    vec<term *> ors;
    for (;;) {
      for (int i = 0; i != ands.n; ++i)
        or_args[i] = and_at(ands[i], j[i]);
      ors.push(mk(Or, or_args));
      for (int i = ands.n;;) {
        if (!i)
          return mk(And, ors);
        --i;
        if (++j[i] < and_size(ands[i]))
          break;
        j[i] = 0;
      }
    }
  }
  default:
    return a;
  }
}

// make clauses

void clausify(term *a) {
  switch (a->tag) {
  case And:
    assert(false);
  case Not:
    neg.push(at(a, 0));
    break;
  case Or:
    for (auto i : a)
      clausify(at(a, i));
    break;
  default:
    pos.push(a);
    break;
  }
}

void clausify_ors(term *a) {
  assert(a->tag != And);
  assert(!neg.n);
  assert(!pos.n);
  clausify(a);
  make_clause();
}
} // namespace

void cnf(term *a) {
  // bind all variables
  get_free_vars(a);
  if (free_vars.n)
    a = mk(All, a, free_vars);

  // negation normal form
  vec<std::pair<term *, term *>> all_vars;
  for (auto x : free_vars)
    all_vars.push(std::make_pair(x, x));
  vec<std::pair<term *, term *>> exists_vars;
  a = nnf(all_vars, exists_vars, true, a);

  // distribute OR into AND
  a = distribute(a);

  // make clauses
  if (a->tag == And)
    for (auto i : a)
      clausify_ors(at(a, i));
  else
    clausify_ors(a);
}
