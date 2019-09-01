#include "main.h"

// globals

type Bool = {Tbool};
type Ind = {Tind};
type Int = {Tint};
type Rat = {Trat};
type Real = {Treal};

// construct

type *mkty(sym *name) { return 0; }

type *mkty(type *ret, const vec<type *> &params) { return 0; }
