#ifndef SCOPE_H
#define SCOPE_H

#include "parse.h"
#include "types.h"

typedef enum {
    DECL_VAR,
    DECL_FUNC,
    DECL_BUILTIN_FUNC,
} DeclType;

typedef struct {
    int is_ref;
    union {
        StrId value;
        StrId *ref;
    };
} VarDecl;

typedef struct {
    StrId *param_names;
    int *param_is_ref;
    size_t param_count;
    Node *body;
    size_t body_len;
} NativeFuncDecl;

typedef struct {
	size_t param_count;
    int *param_is_ref;
    StrId (*func)(BuiltinFnArgList);
} BuiltinFuncDecl;

typedef struct {
    int is_native;
    union {
        NativeFuncDecl native;
        BuiltinFuncDecl builtin;
    };
} FuncDecl;

typedef struct {
    DeclType type;
    StrId ident;

    union {
        VarDecl var;
        FuncDecl func;
    };
} Decl;

typedef struct Scope {
    struct Scope *logical_parent;
    struct Scope *parent;
    Decl *declarations;
    size_t num_declarations;
    size_t capacity;
} Scope;

void scope_resize_if_necessary(Scope *scope);
Scope *scope_make_child(Scope *parent, int logical_child);
Scope *scope_delete(Scope *scope);
void scope_clear(Scope *scope);
Scope scope_make();

#endif
