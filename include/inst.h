#ifndef INST_H
#define INST_H

#include <stddef.h>
#include "attr.h"
#include "arr.h"
#include "fd.h"

typedef struct Inst {
  AttrSet uni;     /* universe, defined attributes */
  FD* fds;         /* defined functional dependencies, dynamic array */
} Inst;

#endif /* INST_H */
