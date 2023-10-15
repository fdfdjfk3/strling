#include "essential.h"
#include "../interner/interner.h"
#include <stdio.h>

StrId SLprint(Interner *interner, StrId str) {
	if (str == NULL) {
		puts("strling print(): invalid str");
		exit(1);
	}
	fwrite(str->ptr, str->len, 1, stdout);	
	return interner_intern_noalloc(interner, "", 0);
}
