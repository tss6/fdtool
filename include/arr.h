#ifndef ARR_H
#define ARR_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* absolute tsoding :)
 * https://www.youtube.com/watch?v=gtk3RZHwJUA */

typedef struct {
  size_t cnt;
  size_t cap;
} hdr_t;

#define ARR_INIT_CAPACITY 1024

/* about why 1.5 as the growth rate:
 * https://stackoverflow.com/questions/1100311/ */

#define arr_push(arr, x)                                                  \
  do {                                                                    \
    if (arr == NULL) {                                                    \
      hdr_t *h_ = malloc(sizeof(*arr)*ARR_INIT_CAPACITY + sizeof(hdr_t)); \
      h_->cnt = 0;                                                        \
      h_->cap = ARR_INIT_CAPACITY;                                        \
      arr = (void*)(h_ + 1);                                              \
    }                                                                     \
    hdr_t *h_ = (hdr_t*)(arr) - 1;                                        \
    if (h_->cnt >= h_->cap) {                                             \
      h_->cap += h_->cap / 2;                                             \
      h_ = realloc(h_, sizeof(*arr)*h_->cap + sizeof(hdr_t));             \
      arr = (void*)(h_ + 1);                                              \
    }                                                                     \
    (arr)[h_->cnt++] = (x);                                               \
  } while(0)

/* memmove [i+1:] on top if [i] */
#define arr_remove(arr, i)                                                \
  do {                                                                    \
    hdr_t *h_ = (hdr_t*)(arr) - 1;                                        \
    memmove((arr) + (i), (arr) + (i) + 1,                                 \
            (--h_->cnt - (i)) * sizeof(*(arr)));                          \
  } while (0)

#define arr_len(arr) ((arr) ? ((hdr_t*)(arr) - 1)->cnt : 0)

#define arr_free(arr) do { if (arr) free((hdr_t*)(arr) - 1); } while (0)

#endif /* ARR_H */
