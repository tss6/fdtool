#include "parser.h"
#include "diag.h"
#include "lexer.h"
#include "arr.h"

#include <string.h>

typedef struct Parser {
  const SourceFile *sf;
  Lexer  lx;
  Tok    tok;      /* curr */
  Tok    prev;
  bool   panic;    /* suppress cascading errors until a sync point */
  bool   failed;
  Span  *spans;    /* one per FD, for semantic errors later (dynamic array) */
} Parser;

#define perr(p, sp, ...)                                     \
   do {                                                      \
    if (!(p)->panic) diag_error((p)->sf, (sp), __VA_ARGS__); \
    (p)->panic  = true;                                      \
    (p)->failed = true;                                      \
  } while (0)

static void advance(Parser *p)
{
  p->prev = p->tok;
  while (1) {
    p->lx.quiet = p->panic;
    p->tok = next(&p->lx);
    if (p->tok.kind != TOK_ERROR) break;
    p->failed = true;               /* lexer already printed error */
    p->panic = true;
  }
}

static bool expect(Parser *p, TokKind k, const char *s)
{
  if (p->tok.kind == k) {
    advance(p);
    return true;
  }

  perr(p, p->tok.span, "expected %s", s);
  return false;
}

static char ident_char(const Parser *p, Tok t)
{
  return *tok_text(p->sf, t);
}

/* skip to the end of the curr decl */
static void sync_decl(Parser *p)
{
  while (p->tok.kind != TOK_EOF && p->tok.kind != TOK_RBRACE)
    advance(p);

  if (p->tok.kind == TOK_RBRACE)
    advance(p);

  p->panic = false;
}

/* skip to the next ',' or '}' inside a list */
static void sync_item(Parser *p)
{
  while (p->tok.kind != TOK_COMMA  &&
         p->tok.kind != TOK_RBRACE &&
         p->tok.kind != TOK_EOF)
    advance(p);
  p->panic = false;
}

/* U = { A, B, C } */
static void parse_universe(Parser *p, Inst *in)
{
  AttrSet set = ATTR_EMPTY;
  Span open = p->tok.span;

  if (!expect(p, TOK_LBRACE, "'{'")) {
    sync_decl(p);
    return;
  }

  while (p->tok.kind == TOK_IDENT) {
    AttrSet a = attr_char(ident_char(p, p->tok));

    if (!attr_disjoint(set, a)) {
      diag_warning(p->sf, p->tok.span, "duplicate attribute '%c' in universe",
          ident_char(p, p->tok));
    }

    set = attr_union(set, a);
    advance(p);

    if (p->tok.kind != TOK_COMMA) break;

    advance(p);
    if (p->tok.kind != TOK_IDENT) {
      perr(p, p->tok.span, "expected attribute after ','");
      sync_decl(p);
      in->uni = set;
      return;
    }
  }

  if (!expect(p, TOK_RBRACE, "',' or '}'")) {
    sync_decl(p);
    in->uni = set;
    return;
  }

  if (attr_empty(set)) {
    diag_error(p->sf, span(open.lo, p->prev.span.hi), "universe cannot be empty");
    p->failed = true;
  }
  in->uni = set;
}

/* one side of an FD: IDENT+ */
static AttrSet parse_side(Parser *p)
{
  AttrSet s = ATTR_EMPTY;
  while (p->tok.kind == TOK_IDENT) {
    s = attr_union(s, attr_char(ident_char(p, p->tok)));
    advance(p);
  }
  return s;
}

/* AB -> CD */
static bool parse_fd(Parser *p, Inst *in)
{
  size_t lo = p->tok.span.lo;

  AttrSet l = parse_side(p);
  if (attr_empty(l)) {
    if (p->tok.kind == TOK_ARROW)
      perr(p, p->tok.span, "expected attribute(s) before '->'");
    else
      perr(p, p->tok.span, "expected a functional dependency, like 'A->B'");
    return false;
  }

  if (!expect(p, TOK_ARROW, "'->'")) return false;

  AttrSet r = parse_side(p);
  if (attr_empty(r)) {
    perr(p, p->tok.span, "expected attribute(s) after '->'");
    return false;
  }

  FD fd = { l, r };
  Span sp = span(lo, p->prev.span.hi);
  arr_push(in->fds, fd);
  arr_push(p->spans, sp);
  return true;
}

/* F = { A->B, ... } */
static void parse_fds(Parser *p, Inst *in)
{
  if (!expect(p, TOK_LBRACE, "'{'")){
    sync_decl(p);
    return;
  }

  if (p->tok.kind != TOK_RBRACE) {
    while (1) {

      if (!parse_fd(p, in))
        sync_item(p);

      if (p->tok.kind != TOK_COMMA) break;

      advance(p);

      p->panic = false;
    }
  }

  if (!expect(p, TOK_RBRACE, "',' or '}'"))
    sync_decl(p);
}

static void parse_decl(Parser *p, Inst *in, bool *have_u, bool *have_f)
{
  Tok head = p->tok;
  char c = head.kind == TOK_IDENT ? ident_char(p, head) : 0;

  if (c != 'U' && c != 'F') {
    perr(p, head.span, "expected 'U' or 'F' to start a declaration");
    if (head.kind != TOK_EOF) advance(p);
    sync_decl(p);
    return;
  }
  advance(p);

  if (!expect(p, TOK_EQUALS, "'=' after declaration name")) {
    sync_decl(p);
    return;
  }

  bool *seen = (c == 'U') ? have_u : have_f;
  if (*seen)
    perr(p, head.span, "'%c' is declared more than once", c);
  *seen = true;

  if (c == 'U') parse_universe(p, in);
  else          parse_fds(p, in);
}

static void validate(Parser *p, const Inst *in)
{
  AttrSet done = ATTR_EMPTY;

  for (size_t i = 0; i < arr_len(in->fds); i++) {
    FD fd = in->fds[i];
    Span sp = p->spans[i];

    /* used in F but not in U */
    AttrSet bad = attr_diff(attr_union(fd.lhs, fd.rhs), in->uni);

    if (!attr_empty(bad)) {

      /* one per attribute */
      for (size_t k = sp.lo; k < sp.hi; k++) {
        char ch = p->sf->contents[k];
        AttrSet a  = attr_char(ch);

        if (attr_empty(a) || attr_disjoint(a, bad) || !attr_disjoint(a, done))
          continue;

        done = attr_union(done, a);

        diag_error(p->sf, span(k, k + 1),
            "attribute '%c' is not in the universe :/", ch);

        p->failed = true;
      }

    } else if (attr_subset(fd.rhs, fd.lhs)) {
      diag_warning(p->sf, sp, "trivial dependency :p");
    }
  }
}

bool parse_inst(const SourceFile *sf, Inst *out)
{
  Parser p;
  memset(&p, 0, sizeof p);
  memset(out, 0, sizeof *out);
  p.sf = sf;
  lexer_init(&p.lx, sf);
  advance(&p);

  bool have_u = false, have_f = false;
  while (p.tok.kind != TOK_EOF) {
    p.panic = false;
    parse_decl(&p, out, &have_u, &have_f);
  }

  p.panic = false;
  if (!have_u)
    perr(&p, p.tok.span, "missing universe declaration 'U = { ... }'");

  p.panic = false;
  if (!have_f)
    perr(&p, p.tok.span, "missing dependencies declaration 'F = { ... }'");

  if (have_u)
    validate(&p, out);

  arr_free(p.spans);
  if (p.failed) {
    inst_free(out);
    return false;
  }
  return true;
}

void inst_free(Inst *in)
{
  arr_free(in->fds);
  memset(in, 0, sizeof *in);
}
