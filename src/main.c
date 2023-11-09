#include "cli/args.h"
#include "globals.h"
#include "interpret.h"
#include "parse.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    g_interner_init();
    args_store_argv(argc, argv);
    const char *initial_file_name = args_get_filename();
	if (initial_file_name == NULL) {
		puts("No file name provided in arguments.");
		exit(EXIT_FAILURE);
	}

	Node *ast = parse_file(initial_file_name);
	interpret(ast);

    // Node *ast = test();
    // interpret(ast);
}
