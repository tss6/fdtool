#include "ops.h"
#include "attr.h"

AttrSet closure(AttrSet x, const FD *fds)
{
  bool upd = true;
  while (upd) {
    upd = false;
    for (size_t i = 0; i< arr_len(fds); i++) {
      /* x >= lhs (have all in lhs) and |rhs - x| != 0 */
      if (attr_subset(fds[i].lhs, x) && !attr_subset(fds[i].rhs, x)) {
        x = attr_union(x, fds[i].rhs);
        upd = true;
      }
    }
  }
  return x;
}

FD *min_cover(const Inst *in);

FD *fds_merge(const FD *fds);

AttrSet *candidate_keys(const Inst *in);

void print_attrs(FILE *out, AttrSet s, bool nl)
{
  if (attr_empty(s)) return;

  for (unsigned i = 0; i < ATTR_COUNT; i++)
    if (attr_in(i, s)) fputc('A' + (int)i, out);

  if (nl) fputc('\n', out);
}

void print_fds(FILE *out, const FD *fds)
{
  for (size_t i = 0; i < arr_len(fds); i++) {
    fputs(" ", out);
    print_attrs(out, fds[i].lhs, false);
    fputs(" -> ", out);
    print_attrs(out, fds[i].rhs, false);
    fputs("\n", out);
  }
}
