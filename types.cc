#include "main.h"

// globals

type Bool = {y_bool};
type Ind = {y_ind};
type Int = {y_int};
type Rat = {y_rat};
type Real = {y_real};

// construct

type *make_type(sym *name) { return 0; }

type *make_type(type *ret, const vec<type *> &params) { return 0; }
