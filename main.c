#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "lexer.h"
#include "source.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s <file>\n", argv[0]);
    return 2;
  }

  SourceFile sf;
  if (!source_load(&sf, argv[1])) {
    fprintf(stderr, "%s: cannot read file: %s\n", argv[1], strerror(errno));
    return 1;
  }

  Lexer lx;
  lexer_init(&lx, &sf);

  for (;;) {
    Tok t = next(&lx);
    if (t.kind == TOK_ERROR) break;

    LineCol lc = source_line_col(&sf, t.span.lo);
    printf("%zu:%zu\t%-7s '%.*s'\n", lc.line, lc.col, tok_name(t.kind),
        (int)span_len(t.span), tok_text(&sf, t));

    if (t.kind == TOK_EOF) break;
  }

  source_free(&sf);
  return 0;
}
