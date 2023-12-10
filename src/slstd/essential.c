#include "essential.h"
#include "../cli/args.h"
#include "../globals.h"
#include "helpful.h" // for the expect arg macros
#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 2048

StrId SLprint(BuiltinFnArgList args) {
    $EXPECT_STRID_ARG(str, 0);

    fwrite(str->ptr, str->len, 1, stdout);
    return get_strid_empty();
}

StrId SLprintln(BuiltinFnArgList args) {
    $EXPECT_STRID_ARG(str, 0);

    fwrite(str->ptr, str->len, 1, stdout);
    printf("\n");
    return get_strid_empty();
}

StrId SLgetchar(BuiltinFnArgList args) {
    char buf[1];
    buf[0] = (char)getchar();
    return g_interner_intern(buf, 1);
}

StrId SLgetline(BuiltinFnArgList args) {
    char buf[MAX_LINE_LEN];
    fgets(buf, MAX_LINE_LEN, stdin);
    size_t len = strlen(buf);

    if (len == 0) {
        return get_strid_empty();
    } else {
        // scrape trailing newline from terminal input
        return g_interner_intern(buf, len - 1);
    }
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

StrId SLpop(BuiltinFnArgList args) {
    $EXPECT_STRID_REF_ARG(str, 0);

    if ((*str)->len == 0) {
        return get_strid_empty();
    }

    const char *pop = (*str)->ptr + (*str)->len - 1;
    *str = g_interner_intern((*str)->ptr, (*str)->len - 1);
    return g_interner_intern(pop, 1);
}

StrId SLpopl(BuiltinFnArgList args) {
    $EXPECT_STRID_REF_ARG(str, 0);

    if ((*str)->len == 0) {
        return get_strid_empty();
    }

    const char *pop = (*str)->ptr;
    *str = g_interner_intern((*str)->ptr + 1, (*str)->len - 1);
    return g_interner_intern(pop, 1);
}

StrId SLpop_substr(BuiltinFnArgList args) {
    $EXPECT_STRID_REF_ARG(str, 0);
    $EXPECT_STRID_ARG(delim, 1);

    if ((*str)->len == 0 || delim->len == 0) {
        return get_strid_empty();
    }

    bool hit_delim = false;
    size_t len_to_pop = 0;

    for (len_to_pop = 0; len_to_pop <= (*str)->len - delim->len; len_to_pop++) {
        if (memcmp((*str)->ptr + ((*str)->len - 1 - len_to_pop), delim->ptr,
                   delim->len) == 0) {
            hit_delim = true;
            break;
        }
    }

    size_t remaining_len =
        (hit_delim) ? (*str)->len - (len_to_pop + delim->len) : 0;

    StrId ret =
        g_interner_intern((*str)->ptr + ((*str)->len - len_to_pop), len_to_pop);

    *str = (hit_delim) ? g_interner_intern((*str)->ptr, remaining_len)
                       : get_strid_empty();

    return ret;
}

StrId SLpopl_substr(BuiltinFnArgList args) {
    $EXPECT_STRID_REF_ARG(str, 0);
    $EXPECT_STRID_ARG(delim, 1);

    if ((*str)->len == 0 || delim->len == 0) {
        return get_strid_empty();
    }

    bool hit_delim = false;
    size_t len_to_pop = 0;

    for (len_to_pop = 0; len_to_pop <= (*str)->len - delim->len; len_to_pop++) {
        if (memcmp((*str)->ptr + len_to_pop, delim->ptr, delim->len) == 0) {
            hit_delim = true;
            break;
        }
    }

    size_t remaining_len =
        (hit_delim) ? (*str)->len - (len_to_pop + delim->len) : 0;

    StrId ret = g_interner_intern((*str)->ptr, len_to_pop);

    *str = (hit_delim)
               ? g_interner_intern((*str)->ptr + len_to_pop + delim->len,
                                   remaining_len)
               : get_strid_empty();

    return ret;
}

StrId SLrev(BuiltinFnArgList args) {
    $EXPECT_STRID_ARG(str, 0);

    char *new = malloc(str->len);
	if (new == NULL) {
		puts("Allocation error in SLrev");
		exit(EXIT_FAILURE);
	}
    for (size_t i = 0; i < str->len; i++) {
        new[str->len - (i + 1)] = str->ptr[i];
    }

    return g_interner_intern_noalloc(new, str->len);
}

StrId SLreplace(BuiltinFnArgList args) {
    $EXPECT_STRID_ARG(str, 0);
    $EXPECT_STRID_ARG(replace, 1);
    $EXPECT_STRID_ARG(with, 2);

    if (replace == with || replace->len == 0 || str->len == 0) {
        return str;
    }

    // Maximum possible size that the output string could be.
    char *buffer = malloc(str->len * with->len);
    if (buffer == NULL) {
        puts("Allocation error in SLreplace");
        exit(EXIT_FAILURE);
    }
    size_t len = 0;

    for (size_t i = 0; i < str->len; i++) {
        if (i <= (str->len - replace->len) &&
            memcmp(str->ptr + i, replace->ptr, replace->len) == 0) {

            memcpy(buffer + len, with->ptr, with->len);
            i += replace->len - 1;
            len += with->len;
        } else {
            buffer[len] = str->ptr[i];
            len += 1;
        }
    }

    buffer = realloc(buffer, len);
    if (buffer == NULL) {
        puts("Allocation error in SLreplace");
        exit(EXIT_FAILURE);
    }

    return g_interner_intern_noalloc(buffer, len);
}
