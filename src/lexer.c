#include "../include/lexer.h"
#include "../include/diag.h"

#include <stdio.h>
#include <ctype.h>

/* apparently ctype.h requires unsigned char:
 * https://stackoverflow.com/questions/17975913/does-ctype-h-still-require-unsigned-char */
static unsigned char u(char c) { return (unsigned char)c; }

void lexer_init(Lexer *lx, const SourceFile *sf)
{
  lx->sf  = sf;
  lx->cur = sf->contents;
  lx->end = sf->contents + sf->size;
}

static size_t off(const Lexer *lx, const char *p)
{
  return (size_t)(p - lx->sf->contents);
}

static Tok token(const Lexer *lx, TokKind type, const char *start)
{
  return (Tok){ type, span(off(lx, start), off(lx, lx->cur)) };
}

Tok next(Lexer *lx)
{
  while (lx->cur < lx->end && isspace(u(*lx->cur)))
    lx->cur++;

  const char *start = lx->cur;

  /* EOF is decided by position, not by seeing a '\0' byte. */
  if (lx->cur == lx->end)
    return token(lx, TOK_EOF, start);

  char c = *lx->cur++;
  switch (c) {
    case '=': return token(lx, TOK_EQUALS, start);
    case '{': return token(lx, TOK_LBRACE, start);
    case '}': return token(lx, TOK_RBRACE, start);
    case ',': return token(lx, TOK_COMMA,  start);
    case '-':
      if (lx->cur < lx->end && *lx->cur == '>') {
        lx->cur++;
        return token(lx, TOK_ARROW, start);
      }
      Tok t = token(lx, TOK_ERROR, start);
      diag_error(lx->sf, t.span, "expected '>' after '-' (did you mean '->'?)");
      return t;
    default:
      break;
  }

  if (isupper(u(c)))
    return token(lx, TOK_IDENT, start);

  Tok t = token(lx, TOK_ERROR, start);
  if (isprint(u(c)))
    diag_error(lx->sf, t.span, "unexpected character '%c'", c);
  else
    diag_error(lx->sf, t.span, "unexpected byte 0x%02x", u(c));
  return t;
}

const char *tok_name(TokKind t)
{
  switch (t) {
    case TOK_IDENT:  return "IDENT";
    case TOK_EQUALS: return "EQUALS";
    case TOK_LBRACE: return "LBRACE";
    case TOK_RBRACE: return "RBRACE";
    case TOK_COMMA:  return "COMMA";
    case TOK_ARROW:  return "ARROW";
    case TOK_EOF:    return "EOF";
    case TOK_ERROR:  return "ERROR";
  }
  return "?";
}

const char *tok_text(const SourceFile *sf, Tok tok)
{
  return sf->contents + tok.span.lo;
}
