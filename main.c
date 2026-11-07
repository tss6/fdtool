#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
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

  Inst inst;
  bool ok = parse_inst(&sf, &inst);
  if (ok) {
    printf("ok: %u attributes, %zu dependencies\n",
        attr_size(inst.uni), arr_len(inst.fds));
    inst_free(&inst);
  }

  source_free(&sf);
  return 0;
}
