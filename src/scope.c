#include "scope.h"
#include <stdio.h>

// TODO: error handling for these functions to handle conditions when any of the
// paramteres are null

inline void scope_resize_if_necessary(Scope *scope) {
  if (scope->num_declarations == scope->capacity) {
    scope->capacity *= 2;
	puts("hi");
    Decl *new_decls =
        realloc(scope->declarations, sizeof(Decl) * scope->capacity);
	puts("hi");
    if (new_decls == NULL) {
      puts("Error resizing scope to allow for more vars. Is ur memory OK?");
      exit(1);
    }
    scope->declarations = new_decls;
  }
}

void scope_add_var(Scope *scope, StrId name, StrId value) {
	puts("a");
  Scope *s = scope;

  while (s != NULL) {
    for (size_t i = 0; i < s->num_declarations; i++) {
      Decl decl = s->declarations[i];
      if (decl.type == DECL_VAR) {
        if (decl.ident == name) {
          s->declarations[i].var_value = value;
          return;
        }
      }
    }
    s = s->parent;
  }

  scope_resize_if_necessary(scope);

  scope->declarations[scope->num_declarations] =
      (Decl){.type = DECL_VAR, .ident = name, .var_value = value};
  scope->num_declarations += 1;
}

void scope_add_func(Scope *scope, Node *func) {
  for (size_t i = 0; i < scope->num_declarations; i++) {
    Decl decl = scope->declarations[i];
    if (decl.type == DECL_FUNC) {
      if (decl.ident == func->func_decl.name) {
        puts("shadowing function declaration is not implemented.");
        exit(1);
      }
    }
  }

  scope_resize_if_necessary(scope);

  Decl decl = (Decl){.type = DECL_FUNC, .ident = func->func_decl.name};
  decl.func.body = func->func_decl.body;
  decl.func.body_len = func->func_decl.body_node_count;
  decl.func.param_count = func->func_decl.param_count;
  decl.func.param_names = func->func_decl.param_names;

  scope->declarations[scope->num_declarations] = decl;
  scope->num_declarations += 1;
}

StrId scope_get_var(Scope *scope, StrId name) {
  Scope *s = scope;

  do {
    Decl *decls = s->declarations;
    for (size_t i = 0; i < s->num_declarations; i++) {
      if (decls[i].type == DECL_VAR) {
        if (decls[i].ident == name) {
          return decls[i].var_value;
        }
      }
    }
    s = s->parent;
  } while (s != NULL);
  printf("No such variable %.*s in scope %p\n", (int)name->len, name->ptr,
         scope);
  exit(1);
}

FuncDecl *scope_get_func(Scope *scope, StrId name) {
  Scope *s = scope;

  do {
    Decl *decls = s->declarations;
    for (size_t i = 0; i < s->num_declarations; i++) {
      if (decls[i].type == DECL_VAR) {
        if (decls[i].ident == name) {
          return &decls[i].func;
        }
      }
    }
    s = s->parent;
  } while (s != NULL);
  return NULL;
}

Scope *scope_make_child(Scope *parent) {
  Scope *child = malloc(sizeof(Scope));
  child->capacity = 10;
  child->declarations = malloc(sizeof(Decl) * 10);
  child->parent = parent;
  child->num_declarations = 0;

  return child;
}

Scope scope_make() {
	return (Scope){
		.num_declarations = 0,
			.parent = NULL,
			.declarations = malloc(sizeof(Decl) * 10),
			.capacity = 10,
	};
}

Scope *scope_delete(Scope *scope) {
  Scope *parent = scope->parent;
  free(scope->declarations);
  free(scope);
  return parent;
}

void scope_clear(Scope *scope) {
	scope->num_declarations = 0;
}
