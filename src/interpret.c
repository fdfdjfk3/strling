#include "interpret.h"
#include "scope.h"
#include "std/sl_stdlib.h"
#include "string.h"
#include <stdarg.h>
#include <stdio.h>

typedef enum {
  CTX_NONE,
  CTX_FUNC,
} InterpretContext;

typedef struct {
  StrId name;
  StrId value;
} StackNode;

typedef struct {
  Scope global_scope;
  Scope *current_scope;
  Interner *interner;
} Machine;

StrId eval_term(Expr *expr) {}

StrId eval_expr(Machine *machine, Expr *expr) {
  switch (expr->type) {
  case EXPR_VALUE:
    return expr->value;
    break;
  case EXPR_IDENT:
    return scope_get_var(machine->current_scope, expr->ident);
    break;
  case EXPR_TERM:
    if (0) {
    }
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
      scope_add_var(machine->current_scope, left, right);
      return interner_intern(machine->interner, "", 0);

    } else if (op == OP_CONCAT) {
      left = eval_expr(machine, expr->term.left);
      right = eval_expr(machine, expr->term.right);
      return str_concat(machine->interner, left, right);

    } else {
      puts("unimplemented");
      exit(1);
    }
	break;
  case EXPR_CALL:
    if (0) {
    }
    StrId name = expr->call.name;
    SLprint(machine->interner, expr->call.args[0].value);
    break;

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
    if (0) {
    }
    eval_expr(machine, ast->top_expr);
    break;
  default:
    puts("unimplemented");
    exit(1);
  }
}

void interpret(Interner *interner, Node *ast) {
  Scope global_scope =
      (Scope){.parent = NULL, .declarations = NULL, .num_declarations = 0};
  Machine machine = (Machine){.interner = interner,
                              .global_scope = global_scope,
                              .current_scope = &global_scope};
  interpret_node(&machine, ast);
}
