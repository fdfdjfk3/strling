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

    } else if (op == OP_EQ) {
      left = eval_expr(machine, expr->term.left);
      right = eval_expr(machine, expr->term.right);
      return (left == right) ? interner_intern(machine->interner, "true", 4)
                             : interner_intern(machine->interner, "false", 5);

    } else if (op == OP_NOT_EQ) {
      left = eval_expr(machine, expr->term.left);
      right = eval_expr(machine, expr->term.right);
      return (left != right) ? interner_intern(machine->interner, "true", 4)
                             : interner_intern(machine->interner, "false", 5);

    } else {
      puts("unimplemented");
      exit(1);
    }
    break;
  case EXPR_CALL:
    if (0) {
    }
    StrId name = expr->call.name;
    return SLprint(machine->interner, eval_expr(machine, &expr->call.args[0]));

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
  case NODE_IF:
    if (0) {
    }
    StrId expr = eval_expr(machine, ast->if_statement.expr);
    if (expr == interner_intern_noalloc(machine->interner, "true", 4)) {
      for (size_t i = 0; i < ast->if_statement.body_node_count; i++) {
        interpret_node(machine, &ast->if_statement.body[i]);
      }
    } else {
      for (size_t i = 0; i < ast->if_statement.num_elifs; i++) {
        Node *elif = &ast->if_statement.elifs[i];
        StrId elif_expr = eval_expr(machine, elif->elif_statement.expr);
        if (elif_expr ==
            interner_intern_noalloc(machine->interner, "true", 4)) {
          for (size_t i = 0; i < elif->elif_statement.body_node_count; i++) {
            interpret_node(machine, &elif->elif_statement.body[i]);
          }
		  break;
        }
      }
    }
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
