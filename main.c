#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "arr.h"
#include "ops.h"
#include "parser.h"
#include "source.h"

#define EXIT_OK    0
#define EXIT_PARSE 1
#define EXIT_USAGE 2

static int cmd_check(const Inst *in, AttrSet x)
{
  (void)x; /* no warnings */
  printf("ok: %u attributes, %zu dependencies\n",
      attr_size(in->uni), arr_len(in->fds));
  return 0;
}

static int cmd_closure(const Inst *in, AttrSet x)
{
  print_attrs(stdout, closure(x, in->fds), true);
  return 0;
}

static int cmd_mincover(const Inst *in, AttrSet x)
{
  (void)x; /* no warnings */
  FD *mc = min_cover(in, false);
  print_fds(stdout, mc);
  arr_free(mc);
  return 0;
}

static int cmd_keys(const Inst *in, AttrSet x)
{
  (void)x; /* no warnings */
  AttrSet *ks = keys(in);
  for (size_t i = 0; i < arr_len(ks); i++)
    print_attrs(stdout, ks[i], true);
  arr_free(ks);
  return 0;
}

static int cmd_nf(const Inst *in, AttrSet x)
{
  (void)x; /* no warnings */
  AttrSet *ks = keys(in);
  FD *tv = nf_violations(in, ks, true);
  FD *bv = nf_violations(in, ks, false);

  printf("BCNF: %s\n", bv ? "VIOLATIONS" : "OK");
  printf("3NF: %s\n",  tv ? "VIOLATIONS" : "OK");

  for (size_t i = 0; i < arr_len(bv); i++) {
    fputs("VIOLATION BCNF: ", stdout);
    print_fd(stdout, &bv[i], false);
    fputs(" (", stdout);
    print_attrs(stdout, bv[i].lhs, false);
    fputs(" not superkey)\n", stdout);
  }

  for (size_t i = 0; i < arr_len(tv); i++) {
    fputs("VIOLATION 3NF: ", stdout);
    print_fd(stdout, &tv[i], false);
    fputs(" (", stdout);
    print_attrs(stdout, tv[i].rhs, false);
    fputs(" not prime, ", stdout);
    print_attrs(stdout, tv[i].lhs, false);
    fputs(" not superkey)\n", stdout);
  }

  arr_free(tv);
  arr_free(bv);
  arr_free(ks);
  return 0;
}

enum cmds
{
  CHECK,
  CLOSURE,
  MINCOVER,
  KEYS,
  NORMALFORM,
  __cmds_count
};

typedef struct Command {
  const char *name;
  bool        needs_x;
  int       (*run)(const Inst *, AttrSet); /* fun ptr to runner */
  const char *desc;
} Command;

static const Command commands[__cmds_count] = {
  [CHECK]      = { "check",     false, cmd_check,    "parse and validate the file only"          },
  [CLOSURE]    = { "closure",   true,  cmd_closure,  "closure of the attribute set given by --X" },
  [MINCOVER]   = { "mincover",  false, cmd_mincover, "minimal cover of the dependencies"         },
  [KEYS]       = { "keys",      false, cmd_keys,     "all candidate keys"                        },
  [NORMALFORM] = { "normalform",false, cmd_nf,       "3NF and BCNF check, with violations"       },
};

static void usage(FILE *out, const char *prog)
{
  fprintf(out, "usage: %s <command> --fds <file> [--X <attrs>]\n\n"
               "commands:\n", prog);

  for (size_t i = 0; i < __cmds_count; i++)
    fprintf(out, "  %-11s %s%s\n", commands[i].name, commands[i].desc,
        commands[i].needs_x ? " (needs --X)" : "");

  fputs("\noptions:\n"
          "  --fds <file>   file with the U = {...} and F = {...} declarations\n"
          "  --X <attrs>    attribute set, e.g. --X AB or --X A,B\n"
          "  -h, --help     show this help\n", out);
}

/* accepts "--opt val" and "--opt=val" */
static const char *opt_val(int argc, char **argv, int *i, const char *opt)
{
  const char *a = argv[*i];
  size_t n = strlen(opt);

  if (strncmp(a, opt, n) != 0)
    return NULL;

  if (a[n] == '=')
    return a + n + 1;

  if (a[n] != '\0')
    return NULL;

  if (*i + 1 >= argc) {
    fprintf(stderr, "%s: option '%s' requires a value\n", argv[0], opt);
    exit(EXIT_USAGE);
  }
  return argv[++*i];
}

/* "AB", "A,B", "A B" -> AttrSet
 * returns false for bad chars */
static bool parse_attrs(const char *s, AttrSet *out)
{
  AttrSet set = ATTR_EMPTY;
  for (; *s; s++) {
    if (*s == ',' || *s == ' ')
      continue;

    AttrSet a = attr_char(*s);
    if (attr_empty(a)) {
      fprintf(stderr, "invalid attribute '%c' in --X (expected A..Z)\n", *s);
      return false;
    }

    set = attr_union(set, a);
  }
  *out = set;
  return true;
}

int main(int argc, char **argv)
{
  const char *prog = argv[0];

  /* need something besides just call */
  if (argc < 2) {
    usage(stderr, prog);
    return EXIT_USAGE;
  }

  /* just help */
  if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
    usage(stdout, prog);
    return EXIT_OK;
  }

  /* get subcommand */
  const Command *cmd = NULL;
  for (size_t i = 0; i < __cmds_count; i++)
    if (!strcmp(argv[1], commands[i].name))
      cmd = &commands[i];

  if (!cmd) {
    fprintf(stderr, "%s: unknown command '%s'\n\n", prog, argv[1]);
    usage(stderr, prog);
    return EXIT_USAGE;
  }

  /* fill requirements */
  const char *path = NULL, *xstr = NULL;
  for (int i = 2; i < argc; i++) {
    const char *v;
    /* help somewhere */
    if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
      usage(stdout, prog);
      return EXIT_OK;
    } else if ((v = opt_val(argc, argv, &i, "--fds"))) {
      path = v;
    } else if ((v = opt_val(argc, argv, &i, "--X"))) {
      xstr = v;
    } else {
      fprintf(stderr, "%s: unknown argument '%s'\n", prog, argv[i]);
      return EXIT_USAGE;
    }
  }

  /* validate */
  if (!path) {
    fprintf(stderr, "%s: missing --fds <file>\n", prog);
    return EXIT_USAGE;
  }
  if (cmd->needs_x && !xstr) {
    fprintf(stderr, "%s: '%s' requires --X <attrs>\n", prog, cmd->name);
    return EXIT_USAGE;
  }
  if (!cmd->needs_x && xstr) {
    fprintf(stderr, "%s: '%s' does not take --X\n", prog, cmd->name);
    return EXIT_USAGE;
  }

  /* --X parse */
  AttrSet x = ATTR_EMPTY;
  if (xstr && !parse_attrs(xstr, &x))
    return EXIT_USAGE;

  /* .fds parse */
  SourceFile sf;
  if (!source_load(&sf, path)) {
    fprintf(stderr, "%s: failed to read file: %s\n", path, strerror(errno));
    return EXIT_PARSE;
  }

  Inst inst;
  int rc = EXIT_PARSE;
  if (parse_inst(&sf, &inst)) {
    if (xstr && !attr_subset(x, inst.uni)) {
      fprintf(stderr, "%s: --X contains attributes not in the universe: ", prog);
      print_attrs(stderr, attr_diff(x, inst.uni), true);
    } else {
      /* run! */
      rc = cmd->run(&inst, x);
    }
    inst_free(&inst);
  } /* diag already printed */

  source_free(&sf);
  return rc;
}
