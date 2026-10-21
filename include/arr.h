#ifndef ARR_H
#define ARR_H

#include <stddef.h>
#include <stdlib.h>

/* absolute tsoding :)
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
      hdr->cap += hdr->cap / 2;                                            \
      hdr = realloc(hdr, sizeof(*arr)*hdr->cap + sizeof(hdr_t));           \
      arr = (void*)(hdr + 1);                                              \
    }                                                                      \
    (arr)[hdr->cnt++] = (x);                                               \
  } while(0)

 #define arr_len(arr)  ((arr) ? ((hdr_t*)(arr) - 1)->cnt : 0)

 #define arr_free(arr) do { if (arr) free((hdr_t*)(arr) - 1); } while (0)

#endif /* ARR_H */
