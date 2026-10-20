#ifndef ARR_H
#define ARR_H

#include <stddef.h>
#include <stdlib.h>

/* very good tsoding :)
 * https://www.youtube.com/watch?v=gtk3RZHwJUA */

typedef struct {
  size_t cnt;
  size_t cap;
} hdr_t;

#define ARR_INIT_CAPACITY 1024

/* about why 1.5 as the growth rate:
 * https://stackoverflow.com/questions/1100311/what-is-the-ideal-growth-rate-for-a-dynamically-allocated-array */

#define arr_push(arr, x)                                                   \
  do {                                                                     \
    if (arr == NULL) {                                                     \
      hdr_t *hdr = malloc(sizeof(*arr)*ARR_INIT_CAPACITY + sizeof(hdr_t)); \
      hdr->cnt = 0;                                                        \
      hdr->cap = ARR_INIT_CAPACITY;                                        \
      arr = (void*)(hdr + 1);                                              \
    }                                                                      \
    hdr_t *hdr = (hdr_t*)(arr) - 1;                                        \
    if (hdr->cnt >= hdr->cap) {                                            \
      hdr->cap *= 1.5f;                                                    \
      hdr = realloc(hdr, sizeof(*arr)*hdr->cap + sizeof(hdr_t));           \
      arr = (void*)(hdr + 1);                                              \
    }                                                                      \
    (arr)[hdr->cnt++] = (x);                                               \
  } while(0)

#define arr_len(arr) ((hdr_t*)(arr) - 1)->cnt

#define arr_free(arr) free((hdr_t*)(arr) - 1)

#endif /* ARR_H */
