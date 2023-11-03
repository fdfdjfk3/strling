#include "error.h"
#include "interner/types.h"
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

// TODO: delete
// static Errors error_logger = (Errors){.num = 0, .list = NULL};
static int num_errors = 0;

Slice get_full_srcline(const char *origin, size_t index) {
    size_t start = index;
    size_t end = index;
    while (start > 0) {
        if (origin[start] == '\n') {
            start += 1;
            break;
        }
        start -= 1;
    }
    while (origin[end] != '\0') {
        if (origin[end] == '\n') {
            break;
        }
        end += 1;
    }

    while (isspace(origin[start])) {
        start += 1;
    }
    while (isspace(origin[end])) {
        end -= 1;
    }

    return (Slice){.len = end - start + 1, .ptr = origin + start};
}

// TODO: delete
/* void error_log(size_t index, size_t row, size_t col, ErrorType type,
               char *error_str) {
    error_logger.num += 1;
    error_logger.list =
        realloc(error_logger.list, error_logger.num * sizeof(Error));
    if (error_logger.list == NULL) {
        puts("Well that's awkward. You see, the program had an error, but then "
             "it had an error while trying to record the error. In this case, "
             "it was a memory reallocation that failed. I don't know why, but "
             "I can provide some information about the error that was going to "
             "be recorded before something stupid happened.");
        printf("%lu|%lu: E%d: %s\n", row, col, type, error_str);
        exit(1);
    }
    error_logger.list[error_logger.num - 1] =
        (Error){.index = index, .row = row, .col = col, .error_str = error_str};
} */

void error_print(const char *source, size_t index, size_t row, size_t col,
                 const char *error_str) {
	num_errors += 1;

    Slice line = get_full_srcline(source, index);
    puts("Error :(");
    printf("%lu | %.*s\n", row, (int)line.len, line.ptr);
    printf("line %lu, column %lu: %s\n", row, col, error_str);
}

void error_printf(const char *source, size_t index, size_t row, size_t col, const char *fmt, ...) {
	num_errors += 1;
	
	Slice line = get_full_srcline(source, index);
	puts("Error :(");
	printf("%lu | %.*s\n", row, (int)line.len, line.ptr);
	printf("line %lu, column %lu: ", row, col);

	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}


int error_get_num_errors() { return num_errors; }

// TODO: delete
/* void error_print_all(const char *origin) {
    for (size_t i = 0; i < error_logger.num; i++) {
        Error error = error_logger.list[i];
        switch (error.type) {
        case ERR_GENERIC: {
            Slice line = get_full_srcline(origin, error.index);
            puts("Error :(");
            printf("%lu | %.*s\n", error.row, (int)line.len, line.ptr);
            printf("line %lu, column %lu: %s\n", error.row, error.col,
                   error.error_str);
            break;
        }
        default:
            puts("bruh");
            exit(1);
        }
    }
}*/
