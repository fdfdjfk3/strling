#include "parse.h"
#include "interpret.h"
#include "interner/interner.h"

int main() {
	Interner interner = interner_new();
	interner_init(&interner);
	Node *ast = test(&interner);
	interpret(&interner, ast);
}

