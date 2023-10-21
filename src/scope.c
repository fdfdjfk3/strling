#include "scope.h"
#include <stdio.h>

// TODO: error handling for these functions to handle conditions when any of the
// paramteres are null

void scope_resize_if_necessary(Scope *scope) {
  if (scope->num_declarations == scope->capacity) {
    scope->capacity *= 2;
    Decl *new_decls =
        realloc(scope->declarations, sizeof(Decl) * scope->capacity);
    if (new_decls == NULL) {
      puts("Error resizing scope to allow for more vars. Is ur memory OK?");
      exit(1);
    }
    scope->declarations = new_decls;
  }
}

Scope *scope_make_child(Scope *parent, int logical_child) {
  Scope *child = malloc(sizeof(Scope));
  child->capacity = 10;
  child->declarations = malloc(sizeof(Decl) * 10);
  child->parent = parent;
  child->logical_parent = (logical_child) ? parent : NULL;
  child->num_declarations = 0;

  return child;
}

Scope scope_make() {
  return (Scope){
      .num_declarations = 0,
      .parent = NULL,
	  .logical_parent = NULL,
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

void scope_clear(Scope *scope) { scope->num_declarations = 0; }
