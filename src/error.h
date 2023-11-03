#ifndef ERROR_H
#define ERROR_H
#include <stdlib.h>

typedef enum {
    ERR_GENERIC,
} ErrorType;

typedef struct {
    size_t index;
    size_t row;
    size_t col;
    const char *error_str;
    ErrorType type;
} Error;

typedef struct {
    size_t num;
    Error *list;
} Errors;

/* void error_log(size_t index, size_t row, size_t col, ErrorType type,
               char *error_str); */
void error_print(const char *source, size_t index, size_t row, size_t col,
                 const char *error_str);
void error_printf(const char *source, size_t index, size_t row, size_t col,
                  const char *fmt, ...);
int error_get_num_errors();
/* void error_print_all(const char *origin); */

#endif
