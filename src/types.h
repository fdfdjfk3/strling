#ifndef TYPES_H
#define TYPES_H
#include "interner/types.h"
#include <stdbool.h>

typedef struct {
	bool is_ref;
	union {
		StrId value;
		StrId *ref;
	};
} BuiltinFnArg;

typedef struct {
    size_t len;
    BuiltinFnArg *list;
} BuiltinFnArgList;

#endif
