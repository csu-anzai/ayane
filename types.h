enum kind_t {
  y_bool,
  y_call,
  y_const,
  y_ind,
  y_int,
  y_rat,
  y_real,
  y_var,
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
type *make_type(sym *name);
type *make_type(type *ret, const vec<type *> &params);
