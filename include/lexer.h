#ifndef LEXER_H
#define LEXER_H

#include "source.h"

typedef enum TokKind {
  TOK_IDENT,    /* A..Z */
  TOK_EQUALS,   /*  =   */
  TOK_LBRACE,   /*  {   */
  TOK_RBRACE,   /*  }   */
  TOK_COMMA,    /*  ,   */
  TOK_ARROW,    /*  ->  */
  TOK_EOF,
  TOK_ERROR
} TokKind;

typedef struct Tok {
  TokKind kind;
  Span    span;
} Tok;

typedef struct Lexer {
  const SourceFile *sf;
  const char       *cur;
  const char       *end;
  bool              quiet; /* dont print errors */
} Lexer;

/* quiet is used to not print errors like this:
 *
 * examples/proffail.fds:2:16: error: expected '>' after '-' (did you mean '->'?)
 *  2 | F = { 	A->BC, C- >D, BD->E }
 *    |       	       ^
 * examples/proffail.fds:2:18: error: unexpected character '>'
 *  2 | F = { 	A->BC, C- >D, BD->E }
 *    |       	         ^
 * examples/proffail.fds:2:20: error: expected '->'
 *  2 | F = { 	A->BC, C- >D, BD->E }
 *    |                    ^
 *
 * happens when parser gets into panic mode
 */

void lexer_init(Lexer *lx, const SourceFile *sf);
Tok  next(Lexer *lx);

const char *tok_name(TokKind t);
const char *tok_text(const SourceFile *sf, Tok tok);

#endif
