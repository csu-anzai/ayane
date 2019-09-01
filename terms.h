// memory

void initmem(double memlimit);
void *alloc(size_t bytes);
void clearmem();

// terms

enum tag_t {
  Add,
  All,
  And,
  Call,
  Ceil,
  DistinctObj,
  Div,
  DivE,
  DivF,
  DivT,
  Eq,
  Eqv,
  Exists,
  False,
  Floor,
  Id,
  Int,
  IsInt,
  IsRat,
  Less,
  Minus,
  Mul,
  Not,
  Or,
  Rat,
  Real,
  RemE,
  RemF,
  RemT,
  Round,
  Sub,
  ToInt,
  ToRat,
  ToReal,
  True,
  Trunc,
  Var,
};

struct term {
  tag_t tag;
  int n;
  union {
    mpz_t int_val;
    mpq_t rat_val;
    struct {
      type *ty_;
      sym *name;
    };
    term *args[];
  };

  type *ty();
};

// globals
extern term false1;
extern term true1;
inline term *mk(bool b) { return b ? &true1 : &false1; }

// construct
term *atom(tag_t tag, int bytes);
term *constant(type *ty, sym *name);
term *var(type *ty);

term *mk(tag_t tag, term *a);
term *mk(tag_t tag, term *a, term *b);
term *mk(tag_t tag, term *a, term *b, term *c);
term *mk(tag_t tag, term *a, const vec<term *> &v);
term *mk(tag_t tag, const vec<term *> &v);

term *implies(term *a, term *b);

// element access
inline term *at(const term *a, int i) {
  assert(0 <= i);
  assert(i < a->n);
  return a->args[i];
}

// iterators
inline range::iterator begin(const term *a) { return 0; }
inline range::iterator end(const term *a) { return a->n; }

// etc
extern vec<term *> free_vars;
void get_free_vars(term *a);

bool eq(term *a, term *b);

// clauses

struct clause {
  bool dead;
  unsigned short neg_n, n;
  term *literals[];

  // iterators
  range neg() const { return range(0, neg_n); }
  range pos() const { return range(neg_n, size()); }

  // capacity
  char size() const { return n; }
  bool empty() const { return !size(); }
};

// globals
extern bool complete;

// construct
extern vec<term *> neg, pos;
void make_clause();

// element access
inline term *at(const clause *c, int i) {
  assert(0 <= i);
  assert(i < c->size());
  return c->literals[i];
}

// iterators
inline range::iterator begin(const clause *a) { return 0; }
inline range::iterator end(const clause *a) { return a->n; }

// etc
clause *fresh_vars(clause *c);
