#include "scope.h"
#include <stdio.h>

void scope_add_var(Scope *scope, StrId name, StrId value) {
  for (size_t i = 0; i < scope->num_declarations; i++) {
    Decl decl = scope->declarations[i];
    if (decl.type == DECL_VAR) {
      if (decl.ident == name) {
        scope->declarations[i].var_value = value;
        return;
      }
    }
  }
  scope->num_declarations += 1;
  Decl *new_decls =
      realloc(scope->declarations, sizeof(Decl) * scope->num_declarations);
  if (new_decls == NULL) {
    printf("Error allocating extra declaration. Var: %.*s, Value: %.*s\n",
           (int)name->len, name->ptr, (int)value->len, value->ptr);
    exit(1);
  }
  scope->declarations = new_decls;
  scope->declarations[scope->num_declarations - 1] =
      (Decl){.type = DECL_VAR, .ident = name, .var_value = value};
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
  scope->num_declarations += 1;
  Decl *new_decls =
      realloc(scope->declarations, sizeof(Decl) * scope->num_declarations);
  if (new_decls == NULL) {
    printf("Error allocating extra declaration. Func: %.*s\n",
           (int)func->func_decl.name->len, func->func_decl.name->ptr);
    exit(1);
  }
  scope->declarations = new_decls;
  Decl decl = (Decl){.type = DECL_FUNC, .ident = func->func_decl.name};
  decl.func.body = func->func_decl.body;
  decl.func.body_len = func->func_decl.body_node_count;
  decl.func.param_count = func->func_decl.param_count;
  decl.func.param_names = func->func_decl.param_names;
  scope->declarations[scope->num_declarations - 1] = decl;
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
  child->declarations = NULL;
  child->parent = parent;
  child->num_declarations = 0;

  return child;
}

Scope *scope_delete(Scope *scope) {
  Scope *parent = scope->parent;
  free(scope);
  return parent;
}
