#ifndef TYPES_H
#define TYPES_H
#include "interner/types.h"
typedef struct {
    int len;
    struct {
        int is_ref;
        union {
            StrId value;
            StrId *ref;
        };
    } *list;
} BuiltinFnArgList;

#endif
