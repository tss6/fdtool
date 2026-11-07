#include "ops.h"
#include "attr.h"

/* added this variant for its useful in min_cover */
static AttrSet closure_skip(AttrSet x, const FD *fds, size_t skip)
{
  bool upd = true;
  while (upd) {
    upd = false;
    for (size_t i = 0; i< arr_len(fds); i++) {
      if (skip == i) continue;
      /* x >= lhs (have all in lhs) and |rhs - x| != 0 */
      if (attr_subset(fds[i].lhs, x) && !attr_subset(fds[i].rhs, x)) {
        x = attr_union(x, fds[i].rhs);
        upd = true;
      }
    }
  }
  return x;
}

AttrSet closure(AttrSet x, const FD *fds)
{
  return closure_skip(x, fds, SIZE_MAX);
}

/* AB -> ACD => AB -> CD */
static FD *remove_trivial(const Inst *in)
{
  FD *g = NULL;

  for (size_t i = 0; i < arr_len(in->fds); i++) {
    FD fd = in->fds[i];
    for (unsigned a = 0; a < ATTR_COUNT; a++) {

      /* only add to g if not in both sides */
      if (attr_in(a, fd.rhs) && !attr_in(a, fd.lhs)) {
        FD s = { fd.lhs, ATTR(a) };
        arr_push(g, s);
      }

    }
  }

  return g;
}

/* A->B, A->C => A->BC */
static FD *fds_merge(const FD *fds)
{
  FD *out = NULL;
  for (size_t i = 0; i < arr_len(fds); i++) {
    bool found = false;

    for (size_t j = 0; j < arr_len(out); j++) {

      /* found 2 fds with the same lhs, merge it */
      if (attr_equal(out[j].lhs, fds[i].lhs)) {
        out[j].rhs = attr_union(out[j].rhs, fds[i].rhs);
        found = true;
        break;
      }

    }
    if (!found) arr_push(out, fds[i]);
  }

  return out;
}

FD *min_cover(const Inst *in, bool merge)
{
  /* rhs unitario */
  FD *g = remove_trivial(in);

  /* remocao de estranhos
   * X+ == (X-B)+ */
  for (size_t i = 0; i < arr_len(in->fds); i++) {
    for (unsigned b = 0; b < ATTR_COUNT; b++) {
      if (!attr_in(b, g[i].lhs))    continue;
      if (attr_size(g[i].lhs) == 1) continue;

      AttrSet red = attr_del(g[i].lhs, b);

      if (attr_subset(g[i].rhs, closure(red, g)))
        g[i].lhs = red;
    }
  }

  /* remocao de redundantes
   * remove A->X if A appears in X+ without A->X */
  for (size_t i = 0; i < arr_len(g);) {
    if (attr_subset(g[i].rhs, closure_skip(g[i].lhs, g, i)))
      arr_remove(g, i);
    else
      i++;
  }

  if (!merge) return g;

  FD *out = fds_merge(g);
  arr_free(g);
  return out;
}


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
