#include "string.h"
#include "interner/interner.h"
#include "types.h"

// TODO: make concatting a lazy operation to reduce allocations, use like
// a rope data type for this or something.
StrId str_concat(Interner *interner, StrId str1, StrId str2) {
  size_t new_len = str1->len + str2->len;
  char *buf = malloc(sizeof(char) * new_len);
  size_t i = 0;
  for (size_t j = 0; j < str1->len; j++) {
    buf[i] = str1->ptr[j];
    i += 1;
  }
  for (size_t j = 0; j < str2->len; j++) {
    buf[i] = str2->ptr[j];
    i += 1;
  }
  return interner_intern_noalloc(interner, buf, new_len);
}
