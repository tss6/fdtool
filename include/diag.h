#ifndef DIAG_H
#define DIAG_H

#include "source.h"

/* stolen from:
 * https://stackoverflow.com/a/35281548
 *
 * this should give warnings if you use the printf part wrong :)
 * see: https://gcc.gnu.org/onlinedocs/gcc/Common-Attributes.html#index-format */
#if !defined(PRINTFLIKE)
#  if defined(__GNUC__)
#    define PRINTFLIKE(n,m) __attribute__((format(printf,n,m)))
#  else
#    define PRINTFLIKE(n,m)
#  endif /* __GNUC__ */
#endif /* PRINTFLIKE */


/* GCC-style diagnostics, printed to stderr:
 *
 *   file:line:col: error: message
 *       2 | F = { A->->BC }
 *         |        ^~
 */
void diag_error  (const SourceFile *sf, Span span, const char *fmt, ...) PRINTFLIKE(3,4);
void diag_warning(const SourceFile *sf, Span span, const char *fmt, ...) PRINTFLIKE(3,4);
void diag_note   (const SourceFile *sf, Span span, const char *fmt, ...) PRINTFLIKE(3,4);

int diag_error_count(void);

#endif /* DIAG_H */
