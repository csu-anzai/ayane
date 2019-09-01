#ifdef _MSC_VER
// not using exceptions
#pragma warning(disable : 4530)
#endif

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <iterator>
#include <unordered_set>
#include <utility>

#include <gmp.h>

// general
#include "etc.h"
#include "mem.h"

// containers
#include "map.h"
#include "range.h"
#include "set.h"
#include "slice.h"
#include "vec.h"

// specific
#include "keywords.h"
#include "sym.h"

#include "types.h"

#include "terms.h"

#include "cnf.h"

// languages
#include "parsing.h"

#include "dimacs.h"
#include "tptp.h"
