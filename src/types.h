#ifndef TYPES_H
#define TYPES_H
#include "interner/types.h"

typedef struct {
	int is_ref;
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
