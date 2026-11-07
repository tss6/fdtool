#ifndef OPS_H
#define OPS_H

#include <stdio.h>
#include "inst.h"

/* fds is a dynamic array */
AttrSet closure(AttrSet x, const FD *fds);

FD *min_cover(const Inst *in);

FD *fds_merge(const FD *fds);

AttrSet *candidate_keys(const Inst *in);

void print_attrs(FILE *out, AttrSet s, bool nl);
void print_fds(FILE *out, const FD *fds);        /* one per line */

#endif
