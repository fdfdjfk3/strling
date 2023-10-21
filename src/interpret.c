#include "interpret.h"
#include "globals.h"
#include "scope.h"
#include "slstd/sl_stdlib.h"
#include <stdarg.h>
#include <stdio.h>

typedef enum {
  CTX_NONE,
  CTX_FUNC,
} InterpretContext;

typedef struct {
  Scope global_scope;
  Scope *current_scope;
} Machine;

Decl *machine_get_decl_if_exists(Machine *machine, StrId ident, DeclType type) {
  Scope *s = machine->current_scope;

  // check nearest scopes
  while (s != NULL) {
    for (size_t i = 0; i < s->num_declarations; i++) {
      Decl decl = s->declarations[i];
      if (decl.type != type || decl.ident != ident) {
        continue;
      }
      return &s->declarations[i];
    }
    s = s->logical_parent;
  }

  // check global scope
  for (size_t i = 0; i < machine->global_scope.num_declarations; i++) {
    Decl decl = machine->global_scope.declarations[i];
    if (decl.type != type || decl.ident != ident) {
      continue;
    }
    return &machine->global_scope.declarations[i];
  }

  // nope!
  return NULL;
}

void machine_set_var_ref(Machine *machine, StrId name, StrId *ref,
                         int overwrite_ok) {
  Decl *existing_decl = machine_get_decl_if_exists(machine, name, DECL_VAR);
  if (existing_decl != NULL) {
    if (overwrite_ok == 0) {
      printf("Could not shadow or modify variable name '%.*s'\n",
             (int)name->len, name->ptr);
      exit(1);
    }
    existing_decl->var.is_ref = 1;
    existing_decl->var.ref = ref;
    return;
  }

  Scope *cur = (machine->current_scope == NULL) ? &machine->global_scope
                                                : machine->current_scope;

  scope_resize_if_necessary(cur);
  cur->declarations[cur->num_declarations] =
      (Decl){.type = DECL_VAR,
             .ident = name,
             .var = (VarDecl){.is_ref = 1, .ref = ref}};
  cur->num_declarations += 1;
}

void machine_set_var(Machine *machine, StrId name, StrId value,
                     int overwrite_ok) {
  Decl *existing_decl = machine_get_decl_if_exists(machine, name, DECL_VAR);
  if (existing_decl != NULL) {
    if (overwrite_ok == 0) {
      printf("Could not shadow or modify variable name '%.*s'\n",
             (int)name->len, name->ptr);
      exit(1);
    }
    if (existing_decl->var.is_ref == 1) {
      *(existing_decl->var.ref) = value;
    } else {
      existing_decl->var.value = value;
    }
    return;
  }

  Scope *cur = (machine->current_scope == NULL) ? &machine->global_scope
                                                : machine->current_scope;

  scope_resize_if_necessary(cur);
  cur->declarations[cur->num_declarations] =
      (Decl){.type = DECL_VAR,
             .ident = name,
             .var = (VarDecl){.is_ref = 0, .value = value}};
  cur->num_declarations += 1;
}

void machine_add_func(Machine *machine, StrId name, Node *func) {
  Decl *existing_decl = machine_get_decl_if_exists(machine, name, DECL_FUNC);
  if (existing_decl != NULL) {
    printf("function shadowing is disallowed for now, i don't know if that's a "
           "language feature i want to have in this language. problematic "
           "function name: '%.*s'\n",
           (int)name->len, name->ptr);
    exit(1);
  }

  Scope *cur = (machine->current_scope == NULL) ? &machine->global_scope
                                                : machine->current_scope;

  scope_resize_if_necessary(cur);
  cur->declarations[cur->num_declarations] =
      (Decl){.type = DECL_FUNC,
             .ident = name,
             .func = (FuncDecl){.body = func->func_decl.body,
                                .body_len = func->func_decl.body_node_count,
                                .param_count = func->func_decl.param_count,
                                .param_names = func->func_decl.param_names}};
  cur->num_declarations += 1;
}

StrId machine_get_var(Machine *machine, StrId name) {
  Decl *decl = machine_get_decl_if_exists(machine, name, DECL_VAR);
  if (decl == NULL) {
    printf("Attempted to access variable with name '%.*s', but it didn't exist "
           "in the current scope or a parent scope.\n",
           (int)name->len, name->ptr);
    exit(1);
  }
  if (decl->var.is_ref) {
    return *decl->var.ref;
  } else {
    return decl->var.value;
  }
}

StrId *machine_get_var_ref(Machine *machine, StrId name) {
  Decl *decl = machine_get_decl_if_exists(machine, name, DECL_VAR);
  if (decl == NULL) {
    printf("Attempted to access variable with name '%.*s', but it didn't exist "
           "in the current scope or a parent scope.\n",
           (int)name->len, name->ptr);
    exit(1);
  }
  if (decl->var.is_ref) {
    return decl->var.ref;
  } else {
    return &decl->var.value;
  }
}

FuncDecl machine_get_func(Machine *machine, StrId name) {
  Decl *decl = machine_get_decl_if_exists(machine, name, DECL_FUNC);
  if (decl == NULL) {
    printf("Tried to access function '%.*s', but it didn't exist in the "
           "current scope or a parent scope.\n",
           (int)name->len, name->ptr);
    exit(1);
  }
  return decl->func;
}

void interpret_node(Machine *machine, Node *ast);

StrId eval_expr(Machine *machine, Expr *expr) {
  switch (expr->type) {
  case EXPR_VALUE:
    return expr->value;
    break;
  case EXPR_IDENT:
    return machine_get_var(machine, expr->ident);
    break;
  case EXPR_TERM: {
    OpType op = expr->term.op;
    StrId left;
    StrId right;

    if (op == OP_SET) {
      if (expr->term.left->type != EXPR_IDENT) {
        puts("Can't assign to non-identifier");
        exit(1);
      }
      left = expr->term.left->ident;
      right = eval_expr(machine, expr->term.right);
      machine_set_var(machine, left, right, 1);
      return g_interner_intern("", 0);

    } else if (op == OP_CONCAT) {
      left = eval_expr(machine, expr->term.left);
      right = eval_expr(machine, expr->term.right);
      return SLconcat(&left, &right);

    } else if (op == OP_EQ) {
      left = eval_expr(machine, expr->term.left);
      right = eval_expr(machine, expr->term.right);
      return (left == right) ? get_strid_true() : get_strid_false();

    } else if (op == OP_NOT_EQ) {
      left = eval_expr(machine, expr->term.left);
      right = eval_expr(machine, expr->term.right);
      return (left != right) ? get_strid_true() : get_strid_false();

    } else if (op == OP_INTERSECTION) {
      left = eval_expr(machine, expr->term.left);
      right = eval_expr(machine, expr->term.right);
      return SLintersect(&left, &right);
    } else {
      puts("unimplemented");
      exit(1);
    }
    break;
  }
  case EXPR_CALL: {
    StrId name = expr->call.name;
    if (name == g_interner_intern("print", 5)) {
      StrId val = eval_expr(machine, &expr->call.args[0]);
      return SLprint(&val);
    }
    FuncDecl func = machine_get_func(machine, name);
    if (expr->call.num_args > func.param_count) {
      printf("Called function '%.*s' with too many args. Called with %d args, "
             "function required %d\n",
             (int)name->len, name->ptr, expr->call.num_args, func.param_count);
      exit(1);
    }
    StrId empty = get_strid_empty();
    machine->current_scope = scope_make_child(machine->current_scope, 0);
    for (size_t i = 0; i < func.param_count; i++) {
      if (i < expr->call.num_args) {
        if (expr->call.args[i].type != EXPR_IDENT) {
          printf(
              "Only identifiers are allowed to be used in mutable function "
              "arguments, as other immediate values don't have a place in "
              "memory. Problematic call: %.*s, Problematic argument: arg %lu",
              (int)expr->call.name->len, expr->call.name->ptr, i);
          exit(1);
        }
        machine_set_var_ref(
            machine, func.param_names[i],
            machine_get_var_ref(machine, expr->call.args[i].ident), 0);
      } else {
        machine_set_var_ref(machine, func.param_names[i], &empty, 0);
      }
    }

    for (size_t i = 0; i < func.body_len; i++) {
      interpret_node(machine, &func.body[i]);
    }
    machine->current_scope = scope_delete(machine->current_scope);
    return get_strid_empty();
  }

  default:
    puts("unimplemented");
    exit(1);
  }
}

void interpret_node(Machine *machine, Node *ast) {
  switch (ast->type) {
  case NODE_PROGRAM:
    for (int i = 0; i < ast->program.program_len; i++) {
      interpret_node(machine, &ast->program.nodes[i]);
    }
    break;

  case NODE_TOP_EXPR:
    eval_expr(machine, ast->top_expr);
    break;

  case NODE_IF: {
    StrId expr = eval_expr(machine, ast->if_statement.expr);
    if (expr == get_strid_true()) {
      machine->current_scope = scope_make_child(machine->current_scope, 1);
      for (size_t i = 0; i < ast->if_statement.body_node_count; i++) {
        interpret_node(machine, &ast->if_statement.body[i]);
      }
      machine->current_scope = scope_delete(machine->current_scope);
    } else {
      for (size_t i = 0; i < ast->if_statement.num_elifs; i++) {
        Node *elif = &ast->if_statement.elifs[i];
        StrId elif_expr = eval_expr(machine, elif->elif_statement.expr);
        if (elif_expr == get_strid_true()) {
          machine->current_scope = scope_make_child(machine->current_scope, 1);
          for (size_t i = 0; i < elif->elif_statement.body_node_count; i++) {
            interpret_node(machine, &elif->elif_statement.body[i]);
          }
          machine->current_scope = scope_delete(machine->current_scope);
          break;
        }
      }
    }
    break;
  }

  case NODE_WHILE: {
    machine->current_scope = scope_make_child(machine->current_scope, 1);
    while (eval_expr(machine, ast->while_statement.expr) == get_strid_true()) {
      for (size_t i = 0; i < ast->while_statement.body_node_count; i++) {
        interpret_node(machine, &ast->if_statement.body[i]);
      }
      scope_clear(machine->current_scope);
    }
    machine->current_scope = scope_delete(machine->current_scope);

    break;
  }

  case NODE_FUNCDECL: {
    machine_add_func(machine, ast->func_decl.name, ast);
    break;
  }

  default:
    puts("unimplemented");
    exit(1);
  }
}

void interpret(Node *ast) {
  Scope global_scope = scope_make();
  Machine machine =
      (Machine){.global_scope = global_scope, .current_scope = NULL};
  interpret_node(&machine, ast);
}
