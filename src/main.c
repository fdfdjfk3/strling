#include "parse.h"
#include "interner/interner.h"

int main() {
	Interner interner = interner_new();
	interner_init(&interner);
	test(&interner);
}

