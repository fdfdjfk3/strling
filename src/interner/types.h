#ifndef TYPES_H
#define TYPES_H

#include <inttypes.h>
#include <stdlib.h>

typedef struct {
  size_t len;
  const char *ptr;
} Slice;
#endif

typedef Slice* StrId;
#define STRID_NULL NULL;

