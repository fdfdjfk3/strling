#include "interner.h"
#include <string.h>
#include <stdio.h>

#define slice(s) s, strlen(s)

int main() {
	Interner i = interner_new();
	interner_init(&i);
	StrId s1 = interner_intern(&i, slice("hello world"));
	StrId s2 = interner_intern(&i, slice("asbestos"));
	StrId s3 = interner_intern(&i, slice("DAH"));
	StrId s4 = interner_intern(&i, slice("hello world"));
	StrId s5 = interner_intern(&i, slice("hrO#@R#@($$#FOflsp[2p32"));

	if (s1 == s4) {
		puts("it worky");
	}
	
	printf("%.*s\n", (int)s3->len, s3->ptr);
}
