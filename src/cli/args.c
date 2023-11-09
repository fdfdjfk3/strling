#include <stdlib.h>
#include "args.h"

static int argc;
static char **argv;

void args_store_argv(int _argc, char **_argv) {
    argc = _argc;
    argv = _argv;
}

char **args_get_argv(int *len) {
	*len = argc;
	return argv;
}

const char *args_get_filename() {
    // start at 1 to skip the executable name
    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (arg[0] == '-') {
			if (arg[1] == '-') {
				break;
			}
            // skip next arg since it's an option not a name
            i += 1;
            continue;
        }
        return arg;
    }
	return NULL;
}
