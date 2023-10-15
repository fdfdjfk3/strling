#include "globals.h"
#include "interner/interner.h"

Interner global_interner;

void g_interner_init() {
	global_interner = interner_new();
	interner_init(&global_interner);
}

StrId g_interner_intern(const char *ptr, size_t len) {
	return interner_intern(&global_interner, ptr, len);	
}

StrId g_interner_intern_noalloc(const char *ptr, size_t len) {
	return interner_intern_noalloc(&global_interner, ptr, len);
}




