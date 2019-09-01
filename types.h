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
extern type bool1;
extern type ind1;
extern type int1;
extern type rat1;
extern type real1;

// construct
type *mkty(sym *name);
type *mkty(type *ret, const vec<type *> &params);
