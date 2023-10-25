#include "globals.h"
#include "interpret.h"
#include "parse.h"

int main() {
    g_interner_init();
    Node *ast = test();
    interpret(ast);
}
