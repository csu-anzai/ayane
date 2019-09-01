#include "main.h"

namespace {
// tokenizer

enum {
  o_dollar_word,
  o_eof,
  o_eqv,
  o_imp,
  o_impr,
  o_nand,
  o_ne,
  o_nor,
  o_term,
  o_var,
  o_word,
  o_xor,
};

struct selection : std::unordered_set<term *> {
  bool all;

  selection(const selection &x) : unordered_set<term *>(x), all(x.all) {}
  selection(bool all) : all(all) {}

  size_t count(term *a) const {
    if (all)
      return 1;
    return unordered_set<term *>::count(a);
  }
};

selection select(true);

void word() {
  auto s = src;
  while (('a' <= *s && *s <= 'z') || ('A' <= *s && *s <= 'Z') ||
         ('0' <= *s && *s <= '9') || *s == '_')
    ++s;
  toksym = intern(src, s - src);
  src = s;
}

void quote() {
  buf.n = 0;
  auto s = src;
  auto q = *s++;
  while (*s != q) {
    if (*s == '\\')
      ++s;
    if (*(unsigned char *)s < ' ')
      err("Unclosed quote");
    buf.push(*s++);
  }
  src = s + 1;
  toksym = intern(buf.p, buf.n);
}

void lex() {
loop:
  auto s = src;
  toksrc = s;
  switch (*s) {
  case ' ':
  case '\f':
  case '\n':
  case '\r':
  case '\t':
  case '\v':
    src = s + 1;
    goto loop;
  case '!':
    switch (s[1]) {
    case '=':
      src = s + 2;
      tok = o_ne;
      return;
    }
    break;
  case '"':
    tok = o_term;
    quote();
    tokterm = atom(t_distinct_object, sizeof(type *) + sizeof(sym *));
    tokterm->name = toksym;
    return;
  case '$':
    src = s + 1;
    tok = o_dollar_word;
    word();
    return;
  case '%':
#ifdef DEBUG
    if (!status)
      for (; *s != '\n'; ++s) {
        for (int i = 1; i != n_szs; ++i)
          if (!memcmp(s, szs[i], strlen(szs[i]))) {
            status = i;
            src = strchr(s, '\n');
            goto loop;
          }
      }
#endif
    src = strchr(s, '\n');
    goto loop;
  case '+':
  case '-':
    if ('0' <= s[1] && s[1] <= '9') {
      tok = o_term;
      parsenum();
      return;
    }
    break;
  case '/':
    if (s[1] != '*') {
      src = s + 1;
      err("Expected '*'");
    }
    for (s += 2; !(s[0] == '*' && s[1] == '/'); ++s)
      if (!*s)
        err("Unclosed comment");
    src = s + 2;
    goto loop;
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    tok = o_term;
    parsenum();
    return;
  case '<':
    switch (s[1]) {
    case '=':
      if (s[2] == '>') {
        src = s + 3;
        tok = o_eqv;
        return;
      }
      src = s + 2;
      tok = o_impr;
      return;
    case '~':
      if (s[2] == '>') {
        src = s + 3;
        tok = o_xor;
        return;
      }
      toksrc = s + 1;
      err("Expected '>'");
    }
    break;
  case '=':
    switch (s[1]) {
    case '>':
      src = s + 2;
      tok = o_imp;
      return;
    }
    break;
  case 'A':
  case 'B':
  case 'C':
  case 'D':
  case 'E':
  case 'F':
  case 'G':
  case 'H':
  case 'I':
  case 'J':
  case 'K':
  case 'L':
  case 'M':
  case 'N':
  case 'O':
  case 'P':
  case 'Q':
  case 'R':
  case 'S':
  case 'T':
  case 'U':
  case 'V':
  case 'W':
  case 'X':
  case 'Y':
  case 'Z':
    tok = o_var;
    word();
    return;
  case '\'':
    tok = o_word;
    quote();
    return;
  case 'a':
  case 'b':
  case 'c':
  case 'd':
  case 'e':
  case 'f':
  case 'g':
  case 'h':
  case 'i':
  case 'j':
  case 'k':
  case 'l':
  case 'm':
  case 'n':
  case 'o':
  case 'p':
  case 'q':
  case 'r':
  case 's':
  case 't':
  case 'u':
  case 'v':
  case 'w':
  case 'x':
  case 'y':
  case 'z':
    tok = o_word;
    word();
    return;
  case '~':
    switch (s[1]) {
    case '&':
      src = s + 2;
      tok = o_nand;
      return;
    case '|':
      src = s + 2;
      tok = o_nor;
      return;
    }
    break;
  case 0:
    tok = o_eof;
    return;
  }
  src = s + 1;
  tok = *s;
}

// parser

bool eat(int o) {
  if (tok == o) {
    lex();
    return true;
  }
  return false;
}

void expect(char o) {
  if (eat(o))
    return;
  char buf[16];
  sprintf(buf, "Expected '%c'", o);
  err(buf);
}

// types

type *read_type() {
  switch (tok) {
  case '(': {
    lex();
    vec<type *> v;
    do
      v.push(read_type());
    while (eat('*'));
    expect(')');
    expect('>');
    return mkty(read_type(), v);
  }
  case o_dollar_word:
    switch (keyword(toksym)) {
    case w_i:
      lex();
      return &Ind;
    case w_int:
      lex();
      return &Int;
    case w_o:
      lex();
      return &Bool;
    case w_rat:
      lex();
      return &Rat;
    case w_real:
      lex();
      return &Real;
    }
    err("unknown word");
  case o_word: {
    auto name = toksym;
    if (name->val && name->is_term)
      err("Expected type");
    lex();
    if (name->val)
      return (type *)name->val;
    auto ty = mkty(name);
    name->val = ty;
    name->is_term = false;
    return ty;
  }
  }
  err("Expected type");
}

void typing() {
  if (eat('(')) {
    typing();
    expect(')');
    return;
  }

  // name
  if (tok != o_word)
    err("Expected role");
  auto name = toksym;
  auto s = toksrc;
  lex();
  expect(':');

  // type: $tType
  if (tok == o_dollar_word && toksym == keywords + w_tType) {
    if (name->val) {
      if (name->is_term) {
        toksrc = s;
        err("Already a term");
      }
    } else {
      name->val = mkty(name);
      name->is_term = false;
    }
    lex();
    return;
  }

  // constant: type
  auto ty = read_type();
  if (!name->val) {
    name->val = constant(ty, name);
    name->is_term = true;
  }
  auto a = (term *)name->val;
  if (a->ty_ != ty) {
    toksrc = s;
    err("already has another type");
  }
}

// terms

term *atomic_term();
term *unitary_formula();
term *logic_formula();

void args(vec<term *> &v) {
  if (!eat('('))
    err("Expected '('");
  do
    v.push(atomic_term());
  while (eat(','));
  if (!eat(')'))
    err("Expected ')'");
}

void args(vec<term *> &v, int arity) {
  args(v);
  if (v.n == arity)
    return;
  char buf[32];
  sprintf(buf, "Expected %d argument", arity);
  if (arity != 1) {
    auto i = strlen(buf);
    buf[i] = 's';
    buf[i + 1] = 0;
  }
  err(buf);
}

term *defined_functor(tag_t tag, int arity) {
  lex();
  vec<term *> v;
  args(v, arity);
  return mk(tag, v);
}

term *atomic_term() {
  switch (tok) {
  case o_dollar_word: {
    vec<term *> v;
    switch (keyword(toksym)) {
    case w_ceiling:
      return defined_functor(t_ceil, 1);
    case w_difference:
      return defined_functor(t_sub, 2);
    case w_distinct: {
      vec<term *> clauses;
      lex();
      args(v);
      for (auto i = v.begin(); i != v.end(); ++i)
        for (auto j = v.begin(); j != i; ++j)
          clauses.push(mk(t_not, mk(t_eq, *i, *j)));
      return mk(t_and, clauses);
    }
    case w_false:
      lex();
      return &false1;
    case w_floor:
      return defined_functor(t_floor, 1);
    case w_greater:
      lex();
      args(v, 2);
      return mk(t_less, v[1], v[0]);
    case w_greatereq:
      lex();
      args(v, 2);
      return mk(t_or, mk(t_less, v[1], v[0]), mk(t_eq, v[1], v[0]));
    case w_is_int:
      return defined_functor(t_is_int, 1);
    case w_is_rat:
      return defined_functor(t_is_rat, 1);
    case w_less:
      return defined_functor(t_less, 2);
    case w_lesseq:
      lex();
      args(v, 2);
      return mk(t_or, mk(t_less, v[0], v[1]), mk(t_eq, v[0], v[1]));
    case w_product:
      return defined_functor(t_mul, 2);
    case w_quotient:
      return defined_functor(t_div, 2);
    case w_quotient_e:
      return defined_functor(t_div_e, 2);
    case w_quotient_f:
      return defined_functor(t_div_f, 2);
    case w_quotient_t:
      return defined_functor(t_div_t, 2);
    case w_remainder_e:
      return defined_functor(t_rem_e, 2);
    case w_remainder_f:
      return defined_functor(t_rem_f, 2);
    case w_remainder_t:
      return defined_functor(t_rem_t, 2);
    case w_round:
      return defined_functor(t_round, 1);
    case w_sum:
      return defined_functor(t_add, 2);
    case w_to_int:
      return defined_functor(t_to_int, 1);
    case w_to_rat:
      return defined_functor(t_to_rat, 1);
    case w_to_real:
      return defined_functor(t_to_real, 1);
    case w_true:
      lex();
      return &true1;
    case w_truncate:
      return defined_functor(t_trunc, 1);
    case w_uminus:
      return defined_functor(t_minus, 1);
    }
    err("unknown word");
  }
  case o_term: {
    auto a = tokterm;
    lex();
    return a;
  }
  case o_var: {
    auto name = toksym;
    if (name->val && !name->is_term)
      err("Expected term");
    lex();

    if (!name->val) {
      name->val = var(&Ind);
      name->is_term = true;
    }

    return (term *)name->val;
  }
  case o_word: {
    auto name = toksym;
    if (name->val && !name->is_term)
      err("Expected term");
    lex();

    if (!name->val) {
      name->val = constant(&Ind, name);
      name->is_term = true;
    }

    if (tok != '(')
      return (term *)name->val;

    vec<term *> v;
    args(v);
    return mk(t_call, (term *)name->val, v);
  }
  }
  err("syntax error");
}

term *infix_unary() {
  auto a = atomic_term();
  switch (tok) {
  case '=':
    lex();
    return mk(t_eq, a, atomic_term());
  case o_ne:
    lex();
    return mk(t_not, mk(t_eq, a, atomic_term()));
  }
  return a;
}

term *quantified(tag_t tag) {
  lex();
  expect('[');
  vec<std::pair<sym *, term *>> old;
  vec<term *> vars;
  do {
    if (tok != o_var)
      err("Expected variable");
    auto name = toksym;
    lex();
    old.push(std::make_pair(name, (term *)name->val));
    auto ty = &Ind;
    if (eat(':'))
      ty = read_type();
    auto a = var(ty);
    name->val = a;
    name->is_term = true;
    vars.push(a);
  } while (eat(','));
  expect(']');
  expect(':');
  auto a = mk(tag, unitary_formula(), vars);
  for (auto i = old.rbegin(); i != old.rend(); ++i)
    i->first->val = i->second;
  return a;
}

term *unitary_formula() {
  switch (tok) {
  case '!':
    return quantified(t_all);
  case '(': {
    lex();
    auto a = logic_formula();
    expect(')');
    return a;
  }
  case '?':
    return quantified(t_exists);
  case '~':
    lex();
    return mk(t_not, unitary_formula());
  }
  return infix_unary();
}

term *logic_formula() {
  auto a = unitary_formula();
  vec<term *> v;
  switch (tok) {
  case '&':
    lex();
    do
      v.push(unitary_formula());
    while (eat('&'));
    return mk(t_and, a, v);
  case '|':
    lex();
    do
      v.push(unitary_formula());
    while (eat('|'));
    return mk(t_or, a, v);
  case o_eqv:
    lex();
    return mk(t_eqv, a, unitary_formula());
  case o_imp:
    lex();
    return implies(a, unitary_formula());
  case o_impr:
    lex();
    return implies(unitary_formula(), a);
  case o_nand:
    lex();
    return mk(t_not, mk(t_and, a, unitary_formula()));
  case o_nor:
    lex();
    return mk(t_not, mk(t_or, a, unitary_formula()));
  case o_xor:
    lex();
    return mk(t_not, mk(t_eqv, a, unitary_formula()));
  }
  return a;
}

void ignore() {
  switch (tok) {
  case '[':
    lex();
    while (!eat(']'))
      ignore();
    break;
  case 0:
    err("Unexpected end of file");
  case '(':
    lex();
    while (!eat(')'))
      ignore();
    break;
  default:
    lex();
    break;
  }
}

term *formula_name() {
  switch (tok) {
  case o_term: {
    auto a = tokterm;
    lex();
    return a;
  }
  case o_word:
    return atomic_term();
  }
  err("Expected formula name");
}

// top level

void read_tptp1(const char *filename);

void annotated_formula() {
  lex();
  expect('(');

  // name
  auto name = formula_name();
  expect(',');

  // role
  if (tok != o_word)
    err("Expected role");
  auto role = toksym;
  if (role == keywords + w_conjecture && conjecture)
    err("multiple conjectures not supported");
  lex();
  expect(',');

  // formula
  if (role == keywords + w_type)
    typing();
  else {
    auto a = logic_formula();
    if (role == keywords + w_conjecture) {
      get_free_vars(a);
      a = mk(t_not, mk(t_all, a, free_vars));
      conjecture = true;
    }
    cnf(a);
  }

  // annotations
  if (eat(','))
    while (tok != ')')
      ignore();

  // end
  expect(')');
  expect('.');
}

void include() {
  auto tptp = getenv("TPTP");
  if (!tptp)
    err("TPTP environment variable not set");
  lex();
  expect('(');

  // filename
  if (tok != o_word)
    err("Expected filename");
  auto n = strlen(tptp);
  vec<char> filename1(n + toksym->n + 2);
  memcpy(filename1.p, tptp, n);
  filename1[n] = '/';
  memcpy(filename1.p + n + 1, toksym->s, toksym->n);
  filename1[n + 1 + toksym->n] = 0;
  lex();

  // select
  auto old_select = select;
  if (eat(',')) {
    expect('[');
    select = selection(false);
    do {
      auto name = formula_name();
      if (old_select.count(name))
        select.insert(name);
    } while (eat(','));
    expect(']');
  }

  // save current token
  auto old_src1 = toksrc;
  auto old_tok = tok;
  auto old_tokString = toksym;
  auto old_tokterm = tokterm;

  // read
  read_tptp1(filename1.p);

  // restore current token
  toksrc = old_src1;
  tok = old_tok;
  toksym = old_tokString;
  tokterm = old_tokterm;

  // restore select
  select = old_select;

  // end
  expect(')');
  expect('.');
}

void read_tptp1(const char *filename) {
  srcfile file(filename);
  lex();
  while (tok != o_eof) {
    if (tok != o_word)
      err("Expected formula");
    switch (keyword(toksym)) {
    case w_cnf:
    case w_fof:
    case w_tff:
      annotated_formula();
      break;
    case w_include:
      include();
      break;
    default:
      err("unknown language");
    }
  }
}
} // namespace

void read_tptp(const char *filename) { read_tptp1(filename); }
