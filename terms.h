// memory

void initmem(double memlimit);
void *alloc(size_t bytes);
void clearmem();

// terms

enum tag_t {
  t_add,
  t_all,
  t_and,
  t_call,
  t_ceil,
  t_distinct_object,
  t_div,
  t_div_e,
  t_div_f,
  t_div_t,
  t_eq,
  t_eqv,
  t_exists,
  t_false,
  t_floor,
  t_id,
  t_int,
  t_is_int,
  t_is_rat,
  t_less,
  t_minus,
  t_mul,
  t_not,
  t_or,
  t_rat,
  t_real,
  t_rem_e,
  t_rem_f,
  t_rem_t,
  t_round,
  t_sub,
  t_to_int,
  t_to_rat,
  t_to_real,
  t_true,
  t_trunc,
  t_var,
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
