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

StrId SLremove_occurrences(StrId str, StrId search) {
    char *buf = malloc(sizeof(char) * str->len);
    size_t buf_len = 0;
    for (size_t i = 0; i <= (str->len - search->len); i++) {
        for (size_t c = 0; c < search->len; c++) {
            if (str->ptr[i + c] != search->ptr[c]) {
                memcpy(buf + buf_len, str->ptr + i, c + 1);
                buf_len += c + 1;
                i += c;
                break;
            }
        }
    }
    buf = realloc(buf, buf_len);
    return g_interner_intern_noalloc(buf, buf_len);
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

StrId SLdifference(StrId str1, StrId str2) {
    size_t max_len = ((str1->len > str2->len) ? str1->len : str2->len);

    char *buf = malloc(sizeof(char) * max_len);
    const char *ptr1 = str1->ptr;
    const char *ptr2 = str2->ptr;
    size_t n = 0;
    for (size_t i = 0; i < max_len; i++) {
        if (i >= str1->len || i >= str2->len) {
            const char *not_at_end = (i >= str1->len) ? ptr2 : ptr1;
            // i is still less than max_len, so it's fine to do this
            while (i < max_len) {
                buf[n] = not_at_end[i];
                i += 1;
                n += 1;
            }
            break;
        } else {
            if (ptr1[i] != ptr2[i]) {
                buf[n] = ptr1[i];
                n += 1;
            }
        }
    }

    buf = realloc(buf, sizeof(char) * n);
    return g_interner_intern_noalloc(buf, n);
}

StrId SLpop(StrId *str) {
    const char *pop = (*str)->ptr + (*str)->len - 1;
    *str = g_interner_intern((*str)->ptr, (*str)->len - 1);
    return g_interner_intern(pop, 1);
}

StrId SLpopl(StrId *str) {
    const char *pop = (*str)->ptr;
    *str = g_interner_intern((*str)->ptr + 1, (*str)->len - 1);
    return g_interner_intern(pop, 1);
}
