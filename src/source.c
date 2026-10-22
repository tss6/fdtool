#include "source.h"
#include "arr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

bool source_load(SourceFile *sf, const char *path)
{
    FILE    *fp = NULL;
    char    *buf = NULL;
    size_t  *starts = NULL;
    size_t  n, i, sz;
    long    size; /* ftell returns long lol */

    memset(sf, 0, sizeof(*sf));
    sf->filename = path;

    if (!(fp = fopen(path, "rb")))
      return false;

    if (fseek(fp, 0, SEEK_END) != 0)
      goto err;

    if ((size = ftell(fp)) < 0 || fseek(fp, 0, SEEK_SET) != 0)
      goto err;

    /* compiler always complaining */
    sz = (size_t)size;

    if (!(buf = malloc(sz + 1)))
      goto err;

    if ((n = fread(buf, 1, sz, fp)) != (size_t)size && ferror(fp))
      goto err;

    fclose(fp);
    buf[n] = '\0';

    /* line 1 starts at 0, every '\n' starts a new line */
    arr_push(starts, 0);
    for (i = 0; i < n; i++)
        if (buf[i] == '\n')
          arr_push(starts, i + 1);

    sf->size     = n;
    sf->contents = buf;
    sf->lstarts  = starts;
    return true;

err:
    if (fp) fclose(fp);
    free(buf);
    memset(sf, 0, sizeof(*sf));
    return false;
}

void source_free(SourceFile *sf)
{
    free(sf->contents);
    arr_free(sf->lstarts);
    memset(sf, 0, sizeof(*sf));
}

LineCol source_line_col(const SourceFile *sf, size_t off)
{
    /* find the last line whose start is <= offset
     * just binary search :p */
    size_t mi, lo = 0, hi = arr_len(sf->lstarts);
    while (hi - lo > 1) {
        mi = lo + (hi - lo) / 2;
        if (sf->lstarts[mi] <= off) lo = mi;
        else                         hi = mi;
    }
    LineCol lc = { lo + 1, off - sf->lstarts[lo] + 1 };
    return lc;
}

const char *source_line_text(const SourceFile *sf, size_t line, size_t *len)
{
    size_t i = line - 1;
    size_t e = sf->size;
    size_t s = sf->lstarts[i];

    if (i + 1 < arr_len(sf->lstarts))
        e = sf->lstarts[i + 1] - 1;

    /* handle CRLF... windows xd */
    if (e > s && sf->contents[e - 1] == '\r') e--;

    *len = e - s;
    return sf->contents + s;
}
