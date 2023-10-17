#include "parse.h"
#include "interpret.h"
#include "globals.h"

int main() {
	g_interner_init();
	Node *ast = test();
	interpret(ast);
}

