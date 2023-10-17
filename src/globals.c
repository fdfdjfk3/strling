#include "globals.h"
#include "interner/interner.h"

Interner global_interner;

StrId strid_true = NULL;
StrId strid_false = NULL;
StrId strid_empty = NULL;

void g_interner_init() {
	global_interner = interner_new();
	interner_init(&global_interner);
	strid_true = interner_intern_noalloc(&global_interner, "true", 4);
	strid_false = interner_intern_noalloc(&global_interner, "false", 5);
	strid_empty = interner_intern_noalloc(&global_interner, "", 0);
}

StrId g_interner_intern(const char *ptr, size_t len) {
	return interner_intern(&global_interner, ptr, len);	
}

StrId g_interner_intern_noalloc(const char *ptr, size_t len) {
	return interner_intern_noalloc(&global_interner, ptr, len);
}

Interner *get_global_interner() {
	return &global_interner;
}

StrId get_strid_true() {
	return strid_true;
}

StrId get_strid_false() {
	return strid_false;
}

StrId get_strid_empty() {
	return strid_empty;
}



