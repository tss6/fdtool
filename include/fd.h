#ifndef FD_H
#define FD_H

#include "attr.h"

/* a functional dependency, such as A -> BC or FK -> U
 * simple enough */

typedef struct FD {
  AttrSet lhs;
  AttrSet rhs;
} FD;

/* no methods for now, will fill if needed */

#endif /* FD_H */
