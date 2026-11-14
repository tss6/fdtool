#ifndef FD_H
#define FD_H

#include "attr.h"

/* a functional dependency, such as A -> BC or FK -> U */

typedef struct FD {
  AttrSet lhs;
  AttrSet rhs;
} FD;

static inline bool fd_ident(const FD* a, const FD* b)
{
  if (a->rhs != b->rhs) return false;
  if (a->lhs != b->lhs) return false;
  return true;
}

#endif /* FD_H */
