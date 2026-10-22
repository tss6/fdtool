#include "diag.h"
#include "colors.h"

#include <stdarg.h>
#include <stdio.h>


typedef enum Severity {
  SEV_ERROR,
  SEV_WARNING,
  SEV_NOTE,
  __sev_count
} Severity;

static const char *labels[__sev_count] = {
  [SEV_ERROR]   = "error",
  [SEV_WARNING] = "warning",
  [SEV_NOTE]    = "note",
};

static const char *colors[__sev_count] = {
  [SEV_ERROR]   = COLOR_RED,
  [SEV_WARNING] = COLOR_PURPLE,
  [SEV_NOTE]    = COLOR_CYAN,
};

static int g_error_count = 0;
int diag_error_count(void) { return g_error_count; }

static int digits(size_t n)
{
  int d = 1;
  while (n >= 10) { n /= 10; d++; }
  return d;
}


/*
 *   file:line:col: error: message          (1)
 *       2 | F = { A->->BC }                (2)
 *         |        ^~                      (3)
 */
static void vreport(Severity sev, const SourceFile *sf, Span span,
    const char *fmt, va_list ap)
{
  use_color();
  const char *sevc  = color ? colors[sev] : "";

  LineCol lc = source_line_col(sf, span.lo);

  /* 1 */
  fprintf(stderr, "%s%s:%zu:%zu:%s %s%s:%s %s", C_BOLD, sf->filename,
      lc.line, lc.col, C_RESET, sevc, labels[sev], C_RESET, C_BOLD);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "%s\n", C_RESET);

  /* 2 */
  size_t len;
  const char *text = source_line_text(sf, lc.line, &len);
  int w = digits(lc.line);
  fprintf(stderr, " %*zu | %.*s\n", w, lc.line, (int)len, text);

  /* 3 */
  fprintf(stderr, " %*s | ", w, "");
  size_t col = lc.col - 1;
  if (col > len) col = len;
  for (size_t i = 0; i < col; i++)
    fputc(text[i] == '\t' ? '\t' : ' ', stderr); /* deal with tabs */

  size_t n = span_len(span);
  if (col + n > len) n = len - col;
  if (n == 0) n = 1;

  fputs(sevc, stderr);
  fputc('^', stderr);
  for (size_t i = 0; i < n; i++) fputc('~', stderr);
  fprintf(stderr, "%s\n", C_RESET);
}

/* all 3 will be the same so... */
#define DEFINE_DIAG(name, sev)                                       \
  void name(const SourceFile *sf, Span span, const char *fmt, ...) { \
    va_list ap;                                                      \
    va_start(ap, fmt);                                               \
    vreport(sev, sf, span, fmt, ap);                                 \
    va_end(ap);                                                      \
    if ((sev) == SEV_ERROR) g_error_count++;                         \
  }

DEFINE_DIAG(diag_note, SEV_NOTE)
DEFINE_DIAG(diag_warning, SEV_WARNING)
DEFINE_DIAG(diag_error, SEV_ERROR)
