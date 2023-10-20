#ifndef SCOPE_H
#define SCOPE_H

#include "types.h"
#include "parse.h"

typedef enum {
	DECL_VAR,
	DECL_FUNC,
} DeclType;

typedef struct {
	StrId *param_names;
	char param_count;
	Node *body;
	size_t body_len;
} FuncDecl;

typedef struct {
	DeclType type;
	StrId ident;

	union {
		StrId var_value;
		FuncDecl func;
	};
} Decl;

typedef struct Scope {
	struct Scope *parent;
	Decl *declarations;
	size_t num_declarations;
	size_t capacity;
} Scope;

void scope_add_var(Scope *scope, StrId name, StrId value);
void scope_add_func(Scope *scope, Node *func);
StrId scope_get_var(Scope *scope, StrId name);
FuncDecl *scope_get_func(Scope *scope, StrId name);
Scope *scope_make_child(Scope *parent);
Scope *scope_delete(Scope *scope);
void scope_clear(Scope *scope);
Scope scope_make();

#endif
