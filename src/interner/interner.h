#ifndef INTERNER_H
#define INTERNER_H

#include "types.h"

typedef struct {
    unsigned int occupied;
    size_t max_entries;
    Slice **entries;
} StrTable;

StrId table_addstr(StrTable *t, Slice str, int should_alloc_str);

typedef struct {
    StrTable table;
} Interner;

Interner interner_new();
void interner_init(Interner *i);
StrId interner_intern(Interner *i, const char *ptr, size_t len);
StrId interner_intern_noalloc(Interner *i, const char *ptr, size_t len);

#endif
