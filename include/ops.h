#ifndef OPS_H
#define OPS_H

#include <stdio.h>
#include "inst.h"

/* fds is a dynamic array */
AttrSet closure(AttrSet x, const FD *fds);

/* returns dynamic array, caller frees */
FD *min_cover(const Inst *in, bool merge);

/* returns dynamic array, caller frees */
AttrSet *keys(const Inst *in);

FD *bcnf(const Inst *in);

void print_attrs(FILE *out, AttrSet s, bool nl);
void print_fds(FILE *out, const FD *fds);        /* one per line */

#endif
