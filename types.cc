#include "main.h"

// globals
type bool1 = {Tbool};
type ind1 = {Tind};
type int1 = {Tint};
type rat1 = {Trat};
type real1 = {Treal};

// construct
type *mkty(sym *name) { return 0; }
type *mkty(type *ret, const vec<type *> &params) { return 0; }
