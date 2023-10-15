#ifndef GLOBALS_H
#define GLOBALS_H

#include "types.h"

void g_interner_init();
StrId g_interner_intern(const char *ptr, size_t len);
StrId g_interner_intern_noalloc(const char *ptr, size_t len);

#endif
