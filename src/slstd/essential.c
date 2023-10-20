#include "essential.h"
#include "../globals.h"
#include <stdio.h>
#include <string.h>

// TODO: figure out a way to not pass in what are essentially `char ***`'s into
// these functions. Strings need to be mutable, and since strings provided by
// the interner (StrId / Slice *) are immutable, the way to mutate a string is
// to change its StrId. but that requires a Slice **.

StrId SLprint(StrId *str) {
  StrId slice = *str;

  fwrite(slice->ptr, slice->len, 1, stdout);
  return get_strid_empty();
}

// TODO: make concatting a lazy operation to reduce allocations, use like
// a rope data type for this or something.
StrId SLconcat(StrId *str1, StrId *str2) {
  StrId slice1 = *str1;
  StrId slice2 = *str2;

  size_t str1_len = slice1->len;
  size_t new_len = str1_len + slice2->len;
  char *buf = malloc(sizeof(char) * new_len);
  strncpy(buf, slice1->ptr, str1_len);
  strncpy(buf + str1_len, slice2->ptr, slice2->len);

  return g_interner_intern_noalloc(buf, new_len);
}

StrId SLintersect(StrId *str1, StrId *str2) {
  StrId slice1 = *str1;
  StrId slice2 = *str2;

  size_t min_len = ((slice1->len < slice2->len) ? slice1->len : slice2->len);

  char *buf = malloc(sizeof(char) * min_len);
  const char *ptr1 = slice1->ptr;
  const char *ptr2 = slice2->ptr;
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
