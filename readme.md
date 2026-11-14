# fdtool

A tool to manipulate functional dependencies.

## Quick Start

```sh
make
./fdtool -h
```

## `.fds` format

### Grammar

The tool uses a specific format to parse functional dependencies (`.fds`). It
follows this grammar:

```txt
file   := decl decl
decl   := 'U' '=' '{' [ attr  (sep attr)* ] '}'
        | 'F' '=' '{' [ fd    (sep fd)*   ] '}'
fd     := attr+ '->' attr+
attr   := 'A' .. 'Z'
sep    := ',' | ';'
```

- `decl`: either `U` (universe) or `F` (fds). each exactly once, any order.
- `attr`: single uppercase letter (`U` and `F` included).
- Spaces, tabs and newlines are ignored between tokens (`->` cannot be split).
- `U = {}` is a error (must have attributes). `F = {}` is allowed.
- Duplicate attributes in `U` give a warning, so does trivial dependencies
  (e.g. `A -> A`).

### Examples

```txt
U = {A,B,C,D,E}
F = { A->BC, C->D, BD->E }
```

```txt
U = {A,B,C,D,E,F}
F = { A->BCDEF, BC->A }
```
same as

```txt
F = { A->F D  C   E     B, BC->   A } U = {A,	B          ;          C,D;E,F}
```

## Commands

There are some inputs [here](exemplos/) that I used to test functionalities.
Here are examples using them:

### Check

Runs (syntactic and semantic) checks on a `.fds` file:

```console
$ ./fdtool check --fds exemplos/lexfail.fds
exemplos/lexfail.fds:2:16: error: expected '>' after '-' (did you mean '->'?)
 2 | F = { 	A->BC, C- >D, BD->E }
   |

$ ./fdtool check --fds exemplos/parfail1.fds
exemplos/parfail1.fds:2:14: warning: trivial dependency :p
 2 | F = { A->BC; C->C; BDG->F }
   |              ^~~~
exemplos/parfail1.fds:2:22: error: attribute 'G' is not in the universe :/
 2 | F = { A->BC; C->C; BDG->F }
   |                      ^
exemplos/parfail1.fds:2:25: error: attribute 'F' is not in the universe :/
 2 | F = { A->BC; C->C; BDG->F }
   |                         ^
```

Diagnostics are printed gcc style as you can see.

### Closure

> Expects the `--X` flag.

Prints the closure of the specified attributes (ordered):

```console
$ ./fdtool closure --fds exemplos/prof.fds --X A
ABCDE

$ ./fdtool closure --X BD --fds exemplos/prof.fds
BDE
```

### Mincover

Prints the minimal cover/set of functional dependencies. This operation's
result is not unique, outputs may differ depending on FD order from `.fds`
file.

> [order1.fds](exemplos/order1.fds) and [order2.fds](exemplos/order2.fds) just
> change F set definition orders

```console
$ ./fdtool mincover --fds exemplos/order1.fds
A->B
B->A
B->C

$ ./fdtool mincover --fds exemplos/order2.fds
A->B
B->A
A->C
```

### Keys

Prints all candidate keys.

```console
$ ./fdtool keys --fds exemplos/multkeys.fds
AB
AC

$ ./fdtool keys --fds exemplos/big.fds
AD
```

### Normalform

Scans schema for 3NF and BCNF violations. Prints whether each NF was violated
and explains which FD(s) failed each check.

```console
$ ./fdtool normalform --fds exemplos/prof1.fds
BCNF: VIOLATIONS
3NF: VIOLATIONS
VIOLATION BCNF: B->C (B not superkey)
VIOLATION 3NF: B->C (C not prime, B not superkey)

$ ./fdtool normalform --fds exemplos/big.fds
BCNF: VIOLATIONS
3NF: VIOLATIONS
VIOLATION BCNF: A->B (A not superkey)
VIOLATION BCNF: B->C (B not superkey)
VIOLATION BCNF: A->C (A not superkey)
VIOLATION BCNF: DE->F (DE not superkey)
VIOLATION 3NF: A->B (B not prime, A not superkey)
VIOLATION 3NF: B->C (C not prime, B not superkey)
VIOLATION 3NF: A->C (C not prime, A not superkey)
VIOLATION 3NF: DE->F (F not prime, DE not superkey)

$ ./fdtool normalform --fds exemplos/multkeys.fds
BCNF: VIOLATIONS
3NF: OK
VIOLATION BCNF: C->B (C not superkey)
```

## References

 - https://stackoverflow.com/questions/7762731/
 - https://stackoverflow.com/questions/109023/
 - https://www.felixcloutier.com/x86/popcnt
 - https://gcc.gnu.org/onlinedocs/gcc/Bit-Operation-Builtins.html#index-_005f_005fbuiltin_005fpopcount
 - https://clang.llvm.org/docs/LanguageExtensions.html#builtin-popcountg
 - https://learn.microsoft.com/en-us/cpp/intrinsics/popcnt16-popcnt-popcnt64?view=msvc-170
 - https://en.wikipedia.org/wiki/Hamming_weight
 - https://www.youtube.com/watch?v=gtk3RZHwJUA
 - https://sourceware.org/glibc/manual/latest/html_node/Feature-Test-Macros.html
 - https://gist.github.com/JBlond/2fea43a3049b38287e5e9cefc87b2124
 - https://stackoverflow.com/questions/1100311/
 - https://stackoverflow.com/a/35281548
 - https://gcc.gnu.org/onlinedocs/gcc/Common-Attributes.html#index-format
 - https://www.inf.ufpr.br/andre/Disciplinas/CI1031-2026-2/03-busca-completa-matematica.pdf
 - https://github.com/rexim/bnfuzzer
 - https://github.com/lotabout/write-a-C-interpreter/blob/master/tutorial/en/3-Lexer.md
 - https://stackoverflow.com/questions/17975913/
 - https://www.geeksforgeeks.org/dbms/how-to-find-minimal-set/
 - https://stackoverflow.com/questions/10284004/
 - Slides DB class

AI was used to research topics and clarify concepts. It as not used to
generate, modify or write any source code in this repository. Each prompt made
I also asked for sources and added them to the references above.

## Notes

This was made for a assignment from my DB classes.

## TODO

- [X] start
- [X] define .fds grammar
- [X] load .fds file
- [X] .fds lexer
- [X] diagnostics :)
- [X] .fds parser
- [X] find a way to encode parsed instance where ops are fast ;-;
    - [X] bitmask?
    - ~graph?~
    - ~both?~
- [X] examples/
- [X] examples for each op
- [X] closure
- [X] mincover
- [X] keys
- [X] normalform
    - [X] 3NF
    - [X] BCNF
- [X] cli
- ~format~
- [X] fix readme
- [X] report
