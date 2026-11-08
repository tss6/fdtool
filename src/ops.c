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
  for (size_t i = 0; i < arr_len(g); i++) {
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


AttrSet *keys(const Inst *in)
{
  AttrSet lall = ATTR_EMPTY, rall = ATTR_EMPTY;
  for (size_t i = 0; i < arr_len(in->fds); i++) {
    lall = attr_union(lall, in->fds[i].lhs);
    /*                      remove trivial... */
    rall = attr_union(rall, attr_diff(in->fds[i].rhs, in->fds[i].lhs));
  }

  /* never derived, in all keys */
  AttrSet source = attr_diff(in->uni, rall);

  /* derived and used, must try */
  AttrSet mid = attr_inter(attr_inter(in->uni, rall), lall);

  /* sinks dont help, so skip */

  AttrSet *keys = NULL;

  /* enumerating all subsets of mid in asc order. if we find a candidate,
   * every key inside it was already found, we can then skip it as it is not
   * minimal */
  AttrSet s = ATTR_EMPTY;
  do {
    AttrSet cand = attr_union(source, s);

    /* min check */
    bool min = true;
    for (size_t i = 0; i < arr_len(keys); i++) {
      if (attr_subset(keys[i], cand)) {
        min = false;
        break;
      }
    }

    /* check if valid key */
    if (min && attr_subset(in->uni, closure(cand, in->fds)))
      arr_push(keys, cand);

    /* competitive programming trick :D
     * we use s = (s - 1) & mask to enumerate submasks of mask, so for
     * 10110 we would generate:
     *
     * 10110  00110
     * 10100  00100
     * 10010  00010
     * 10000  00000
     *
     * see:
     * https://www.inf.ufpr.br/andre/Disciplinas/CI1031-2026-2/03-busca-completa-matematica.pdf
     *
     * but here we start at 0b0, and we want to go up.
     * to do that, because AttrSet is unsigned, we use s = (s - mask) & mask,
     * because, in unsigned arithmetic:
     *
     * s - mask = s + ~mask + 1
     *
     * s is a submask of mask, it then shares no bits with ~mask, so:
     *
     * s + ~mask = s | ~mask
     *
     * and then:
     *
     * (s - mask) & mask == ((s | ~mask) + 1) & mask
     *
     * the +1 carries up to the first in the mask that has not been set yet
     * and sets it. then we & with mask to unset the ~mask bits we set.
     *
     * in the end, after enumerating all submasks, s == mask:
     *
     * s = (s - mask) & mask
     * s = (s - s) & mask
     * s = 0 & mask
     * s = 0 == ATTR_EMPTY
     *
     * B)
     */
    s = (s - mid) & mid;
  } while (s != ATTR_EMPTY);

  return keys;
}

static AttrSet prime_attrs(const AttrSet *ks)
{
  AttrSet p = ATTR_EMPTY;
  for (size_t i = 0; i < arr_len(ks); i++)
    p = attr_union(p, ks[i]);
  return p;
}

FD *nf_violations(const Inst *in, const AttrSet *ks, bool tnf)
{
  FD *cover = min_cover(in, false);   /* nao trivial, rhs unitario */
  AttrSet prime = prime_attrs(ks);
  FD *out = NULL;

  for (size_t i = 0; i < arr_len(cover); i++) {
    FD fd = cover[i];

    /* X is a superkey */
    if (attr_subset(in->uni, closure(fd.lhs, in->fds))) continue;

    /* 3NF: rhs can be prime */
    if (tnf && attr_subset(fd.rhs, prime)) continue;

    arr_push(out, fd);
  }

  arr_free(cover);
  return out;
}

void print_attrs(FILE *out, AttrSet s, bool nl)
{
  if (attr_empty(s)) return;

  for (unsigned i = 0; i < ATTR_COUNT; i++)
    if (attr_in(i, s)) fputc('A' + (int)i, out);

  if (nl) fputc('\n', out);
}

void print_fd(FILE *out, const FD *fd, bool nl)
{
  print_attrs(stdout, fd->lhs, false);
  /* no spaces huh */
  fputs("->", out);
  print_attrs(stdout, fd->rhs, false);
  if (nl) fputc('\n', out);
}

void print_fds(FILE *out, const FD *fds)
{
  for (size_t i = 0; i < arr_len(fds); i++)
    print_fd(out, &fds[i], true);
}
