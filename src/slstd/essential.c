#include "essential.h"
#include "../globals.h"
#include <stdio.h>
#include <string.h>

StrId SLprint(StrId str) {
    fwrite(str->ptr, str->len, 1, stdout);
    return get_strid_empty();
}

// TODO: make concatting a lazy operation to reduce allocations, use like
// a rope data type for this or something.
StrId SLconcat(StrId str1, StrId str2) {
    size_t str1_len = str1->len;
    size_t new_len = str1_len + str2->len;
    char *buf = malloc(sizeof(char) * new_len);
    memcpy(buf, str1->ptr, str1_len);
    memcpy(buf + str1_len, str2->ptr, str2->len);

    return g_interner_intern_noalloc(buf, new_len);
}

StrId SLintersect(StrId str1, StrId str2) {
    size_t min_len = ((str1->len < str2->len) ? str1->len : str2->len);

    char *buf = malloc(sizeof(char) * min_len);
    const char *ptr1 = str1->ptr;
    const char *ptr2 = str2->ptr;
    size_t n = 0;
    for (size_t i = 0; i < min_len; i++) {
        if (ptr1[i] == ptr2[i]) {
            buf[n] = ptr1[i];
            n += 1;
        }
    }
    buf = realloc(buf, sizeof(char) * n);
    return g_interner_intern_noalloc(buf, n);
}
