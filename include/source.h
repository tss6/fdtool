#ifndef SOURCE_H
#define SOURCE_H

#include <stddef.h>
#include <stdbool.h>

/* intend to support blobs (such as *.fds) and diagnostics later so
 * this will be useful to use already :) */

typedef struct SourceFile {
  const char *filename;    /* not owned so const   */
  char       *contents;    /* owned, \0 at the end */
  size_t     size;         /* in bytes, without \0 */
  size_t     *lstarts;     /* owned, points to first byte of the line, vector */
} SourceFile;

/* [lo, hi) range into file.contents */
typedef struct Span {
  size_t lo;
  size_t hi;
} Span;

typedef struct LineCol {
  size_t line;
  size_t col;
} LineCol;

static inline Span span(size_t lo, size_t hi) { Span s = {lo, hi}; return s; }
static inline size_t span_len(Span s) { return s.hi - s.lo; }

bool source_load(SourceFile *sf, const char *filepath);
void source_free(SourceFile *sf);

LineCol source_line_col(const SourceFile *sf, size_t off);

const char *source_line_text(const SourceFile *sf, size_t line, size_t *len);

#endif /* SOURCE_H */
