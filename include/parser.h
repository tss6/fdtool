#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "inst.h"
#include "source.h"

/* err -> ret false (printed)
 * ok  -> ret true, caller frees out (inst_free) */
bool parse_inst(const SourceFile *sf, Inst *out);
void inst_free(Inst *in);

#endif
