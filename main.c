#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "source.h"
#include "ops.h"

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
  if (!ok) { return 1; }

  print_fds(stdout, inst.fds);

  AttrSet cl = closure(ATTR_A, inst.fds);

  printf("A closure: ");
  print_attrs(stdout, cl, true);

  cl = closure(ATTR_B|ATTR_D, inst.fds);

  printf("BD closure: ");
  print_attrs(stdout, cl, true);


  printf("Min cover:\n");
  FD *mc = min_cover(&inst, false);

  print_fds(stdout, mc);


  printf("Keys:\n");
  AttrSet *ks = keys(&inst);

  for (size_t i = 0; i < arr_len(ks); i++) {
    print_attrs(stdout, ks[i], true);
  }


  printf("3NF:\n");
  FD *v = nf_violations(&inst, ks, true);

  print_fds(stdout, v);
  arr_free(v);


  printf("BCNF:\n");
  v = nf_violations(&inst, ks, false);

  print_fds(stdout, v);

  arr_free(v);
  arr_free(mc);
  arr_free(ks);
  inst_free(&inst);
  source_free(&sf);
  return 0;
}
