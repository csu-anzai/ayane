enum kind_t {
  Tbool,
  Tcall,
  Tconst,
  Tind,
  Tint,
  Trat,
  Treal,
  Tvar,
};

struct type {
  kind_t tag;
  int n;
  union {
    sym *name;
    type *args[];
  };
};

// globals
extern type Bool;
extern type Ind;
extern type Int;
extern type Rat;
extern type Real;

// construct
type *mkty(sym *name);
type *mkty(type *ret, const vec<type *> &params);
