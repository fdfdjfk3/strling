#ifndef GLOBALS_H
#define GLOBALS_H

#include "types.h"
#include "interner/interner.h"

void g_interner_init();
StrId g_interner_intern(const char *ptr, size_t len);
StrId g_interner_intern_noalloc(const char *ptr, size_t len);
Interner *get_global_interner();

StrId get_strid_true();
StrId get_strid_false();
StrId get_strid_empty();

#endif
