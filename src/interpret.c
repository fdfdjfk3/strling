#include "interpret.h"
#include "globals.h"
#include "scope.h"
#include "slstd/sl_stdlib.h"
#include <stdio.h>

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
                         bool overwrite_ok) {
    Decl *existing_decl = machine_get_decl_if_exists(machine, name, DECL_VAR);
    if (existing_decl != NULL) {
        if (overwrite_ok == false) {
            printf("Could not shadow or modify variable name '%.*s'\n",
                   (int)name->len, name->ptr);
            exit(EXIT_FAILURE);
        }
        existing_decl->var.is_ref = true;
        existing_decl->var.ref = ref;
        return;
    }

    Scope *cur = (machine->current_scope == NULL) ? &machine->global_scope
                                                  : machine->current_scope;

    scope_resize_if_necessary(cur);
    cur->declarations[cur->num_declarations] =
        (Decl){.type = DECL_VAR,
               .ident = name,
               .var = (VarDecl){.is_ref = true, .ref = ref}};
    cur->num_declarations += 1;
}

void machine_set_var(Machine *machine, StrId name, StrId value,
                     bool overwrite_ok) {
    Decl *existing_decl = machine_get_decl_if_exists(machine, name, DECL_VAR);
    if (existing_decl != NULL) {
        if (overwrite_ok == false) {
            printf("Could not shadow or modify variable name '%.*s'\n",
                   (int)name->len, name->ptr);
            exit(EXIT_FAILURE);
        }
        if (existing_decl->var.is_ref == true) {
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
               .var = (VarDecl){.is_ref = false, .value = value}};
    cur->num_declarations += 1;
}

void machine_add_func(Machine *machine, StrId name, Node *func) {
    Decl *existing_decl = machine_get_decl_if_exists(machine, name, DECL_FUNC);
    if (existing_decl != NULL) {
        printf("function shadowing is disallowed for now, i don't know if "
               "that's a "
               "language feature i want to have in this language. problematic "
               "function name: '%.*s'\n",
               (int)name->len, name->ptr);
        exit(EXIT_FAILURE);
    }

    Scope *cur = (machine->current_scope == NULL) ? &machine->global_scope
                                                  : machine->current_scope;

    scope_resize_if_necessary(cur);
    cur->declarations[cur->num_declarations] = (Decl){
        .type = DECL_FUNC,
        .ident = name,
        .func = (FuncDecl){.is_native = true,
                           .native = (NativeFuncDecl){
                               .body = func->func_decl.body,
                               .body_len = func->func_decl.body_node_count,
                               .param_count = func->func_decl.param_count,
                               .param_names = func->func_decl.param_names,
                               .param_is_ref = func->func_decl.param_is_ref}}};

    cur->num_declarations += 1;
}

void machine_add_builtin_func(Machine *machine, StrId name,
                              BuiltinFuncDecl decl) {
    Decl *existing_decl =
        machine_get_decl_if_exists(machine, name, DECL_BUILTIN_FUNC);
    if (existing_decl != NULL) {
        printf("function shadowing is disallowed for now, i don't know if "
               "that's a language feature i want to have in this language.");
        exit(EXIT_FAILURE);
    }

    Scope *cur = (machine->current_scope == NULL) ? &machine->global_scope
                                                  : machine->current_scope;

    scope_resize_if_necessary(cur);
    cur->declarations[cur->num_declarations] =
        (Decl){.type = DECL_BUILTIN_FUNC,
               .ident = name,
               .func = (FuncDecl){.is_native = false, .builtin = decl}};
    cur->num_declarations += 1;
}

StrId machine_get_var(Machine *machine, StrId name) {
    Decl *decl = machine_get_decl_if_exists(machine, name, DECL_VAR);
    if (decl == NULL) {
        printf("Attempted to access variable with name '%.*s', but it didn't "
               "exist in the current scope or a parent scope.\n",
               (int)name->len, name->ptr);
        exit(EXIT_FAILURE);
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
        printf("Attempted to get reference to variable with name '%.*s', but "
               "it didn't exist in the current scope or a parent scope.\n",
               (int)name->len, name->ptr);
        exit(EXIT_FAILURE);
    }
    if (decl->var.is_ref) {
        return decl->var.ref;
    } else {
        return &decl->var.value;
    }
}

FuncDecl machine_get_func(Machine *machine, StrId name) {
    Decl *decl = machine_get_decl_if_exists(machine, name, DECL_BUILTIN_FUNC);
    if (decl != NULL) {
        return decl->func;
    }

    decl = machine_get_decl_if_exists(machine, name, DECL_FUNC);

    if (decl == NULL) {
        printf("Tried to access function '%.*s', but it didn't exist in the "
               "current scope or a parent scope.\n",
               (int)name->len, name->ptr);
        exit(EXIT_FAILURE);
    }
    return decl->func;
}

void machine_import_builtin_lib(Machine *machine, BuiltinLib lib) {
#define stringify(x) #x
    switch (lib) {
    case BUILTIN_LIB_ESSENTIAL: {
        static bool ref_0[] = {false};
        static bool ref_1[] = {true};
        static bool ref_1_0[] = {true, false};
        static bool ref_0_0_0[] = {false, false, false};
        machine_add_builtin_func(machine, g_interner_intern("print", 5),
                                 (BuiltinFuncDecl){.func = SLprint,
                                                   .param_count = 1,
                                                   .param_is_ref = ref_0});
        machine_add_builtin_func(machine, g_interner_intern("println", 7),
                                 (BuiltinFuncDecl){.func = SLprintln,
                                                   .param_count = 1,
                                                   .param_is_ref = ref_0});
        machine_add_builtin_func(machine, g_interner_intern("getchar", 7),
                                 (BuiltinFuncDecl){.func = SLgetchar,
                                                   .param_count = 0,
                                                   .param_is_ref = NULL});
        machine_add_builtin_func(machine, g_interner_intern("getline", 7),
                                 (BuiltinFuncDecl){.func = SLgetline,
                                                   .param_count = 0,
                                                   .param_is_ref = NULL});
        machine_add_builtin_func(machine, g_interner_intern("pop", 3),
                                 (BuiltinFuncDecl){.func = SLpop,
                                                   .param_count = 1,
                                                   .param_is_ref = ref_1});
        machine_add_builtin_func(machine, g_interner_intern("popl", 4),
                                 (BuiltinFuncDecl){.func = SLpopl,
                                                   .param_count = 1,
                                                   .param_is_ref = ref_1});
        machine_add_builtin_func(machine, g_interner_intern("pop_substr", 10),
                                 (BuiltinFuncDecl){.func = SLpop_substr,
                                                   .param_count = 2,
                                                   .param_is_ref = ref_1_0});
        machine_add_builtin_func(machine, g_interner_intern("popl_substr", 11),
                                 (BuiltinFuncDecl){.func = SLpopl_substr,
                                                   .param_count = 2,
                                                   .param_is_ref = ref_1_0});
        machine_add_builtin_func(machine, g_interner_intern("rev", 3),
                                 (BuiltinFuncDecl){.func = SLrev,
                                                   .param_count = 1,
                                                   .param_is_ref = ref_0});
        machine_add_builtin_func(machine, g_interner_intern("replace", 7),
                                 (BuiltinFuncDecl){.func = SLreplace,
                                                   .param_count = 3,
                                                   .param_is_ref = ref_0_0_0});
        break;
    }
    default:
        puts("Library " stringify(lib) " unimplemented");
        exit(EXIT_FAILURE);
    }
#undef stringify
}

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
                exit(EXIT_FAILURE);
            }
            left = expr->term.left->ident;
            right = eval_expr(machine, expr->term.right);
            machine_set_var(machine, left, right, 1);
            return get_strid_empty();

        } else if (op == OP_CONCAT) {
            left = eval_expr(machine, expr->term.left);
            right = eval_expr(machine, expr->term.right);
            return SLconcat(left, right);

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
            return SLintersect(left, right);

        } else if (op == OP_DIFFERENCE) {
            left = eval_expr(machine, expr->term.left);
            right = eval_expr(machine, expr->term.right);
            return SLdifference(left, right);

        } else if (op == OP_BOOL_AND) {
            left = eval_expr(machine, expr->term.left);
            right = eval_expr(machine, expr->term.right);
            StrId strid_true = get_strid_true();
            if (left == strid_true && right == strid_true) {
                return strid_true;
            } else {
                return get_strid_false();
            }
        } else if (op == OP_BOOL_OR) {
            left = eval_expr(machine, expr->term.left);
            right = eval_expr(machine, expr->term.right);
            StrId strid_true = get_strid_true();
            if (left == strid_true || right == strid_true) {
                return strid_true;
            } else {
                return get_strid_false();
            }
        } else if (op == OP_REMOVE_OCCUR) {
            left = eval_expr(machine, expr->term.left);
            right = eval_expr(machine, expr->term.right);
            return SLremove_occurrences(left, right);
        } else {
            puts("unimplemented");
            exit(EXIT_FAILURE);
        }
        break;
    }
    case EXPR_CALL: {
        StrId name = expr->call.name;
        FuncDecl func = machine_get_func(machine, name);

        // native functions have to be called differently.
        if (func.is_native == false) {
            BuiltinFnArg args[255];
            if (expr->call.num_args > func.builtin.param_count) {
                printf("Called function '%.*s' with too many args. Called "
                       "with %d "
                       "args, "
                       "function required %lu\n",
                       (int)name->len, name->ptr, expr->call.num_args,
                       func.builtin.param_count);
                exit(EXIT_FAILURE);
            }
            for (size_t i = 0; i < func.builtin.param_count; i++) {
                if (i >= expr->call.num_args) {
                    if (func.builtin.param_is_ref[i]) {
                        printf("Attempted to implicitly pass an empty string "
                               "literal to an "
                               "argument that requires an LVALUE string. "
                               "Problematic call: "
                               "%.*s, Problematic argument: arg %d\n",
                               (int)expr->call.name->len, expr->call.name->ptr,
                               (int)i);
                        exit(EXIT_FAILURE);
                    }
                    args[i] = (BuiltinFnArg){.is_ref = false,
                                             .value = get_strid_empty()};
                    continue;
                }

                if (func.builtin.param_is_ref[i]) {
                    // printf("func: %.*s, %lu: %d\n",
                    // (int)expr->call.name->len, expr->call.name->ptr, i,
                    // func.builtin.param_is_ref[i]);
                    if (expr->call.args[i].type != EXPR_IDENT) {
                        printf("Only identifiers are allowed to be "
                               "used in "
                               "mutable function "
                               "arguments, as other immediate values "
                               "don't "
                               "have a place in "
                               "memory. Problematic call: %.*s, "
                               "Problematic "
                               "argument: arg "
                               "%lu\n",
                               (int)expr->call.name->len, expr->call.name->ptr,
                               i);
                        exit(EXIT_FAILURE);
                    }
                    args[i] =
                        (BuiltinFnArg){.is_ref = true,
                                       .ref = machine_get_var_ref(
                                           machine, expr->call.args[i].ident)};
                    continue;
                }

                args[i] = (BuiltinFnArg){
                    .is_ref = false,
                    .value = eval_expr(machine, &expr->call.args[i])};
                continue;
            }

            // Call the function with the right args.
            // printf("%lu\n", func.builtin.param_count);
            return func.builtin.func((BuiltinFnArgList){
                .len = func.builtin.param_count, .list = args});
        }

        if (expr->call.num_args > func.native.param_count) {
            printf("Called function '%.*s' with too many args. Called "
                   "with %d "
                   "args, "
                   "function required %lu\n",
                   (int)name->len, name->ptr, expr->call.num_args,
                   func.native.param_count);
            exit(EXIT_FAILURE);
        }
        StrId empty = get_strid_empty();
        machine->current_scope = scope_make_child(machine->current_scope, 0);
        for (size_t i = 0; i < func.native.param_count; i++) {
            if (i < expr->call.num_args) {
                if (func.native.param_is_ref[i]) {
                    if (expr->call.args[i].type != EXPR_IDENT) {
                        printf("Only identifiers are allowed to be "
                               "used in "
                               "mutable function "
                               "arguments, as other immediate values "
                               "don't "
                               "have a place in "
                               "memory. Problematic call: %.*s, "
                               "Problematic "
                               "argument: arg "
                               "%lu\n",
                               (int)expr->call.name->len, expr->call.name->ptr,
                               i);
                        exit(EXIT_FAILURE);
                    } else {
                        machine_set_var_ref(
                            machine, func.native.param_names[i],
                            machine_get_var_ref(machine,
                                                expr->call.args[i].ident),
                            false);
                    }
                } else {
                    machine_set_var(machine, func.native.param_names[i],
                                    eval_expr(machine, &expr->call.args[i]),
                                    false);
                }
            } else {
                if (func.native.param_is_ref[i]) {
                    printf("Attempted to implicitly pass an empty string "
                           "literal to an "
                           "argument that requires an LVALUE string. "
                           "Problematic call: "
                           "%.*s, Problematic argument: arg %lu\n",
                           (int)expr->call.name->len, expr->call.name->ptr, i);
                    exit(EXIT_FAILURE);
                }
                machine_set_var_ref(machine, func.native.param_names[i], &empty,
                                    false);
            }
        }

        for (size_t i = 0; i < func.native.body_len; i++) {
            interpret_node(machine, &func.native.body[i]);
            if (machine->return_pending != NULL) {
                StrId ret = machine->return_pending;
                machine->return_pending = STRID_NULL;
                machine->current_scope = scope_delete(machine->current_scope);
                return ret;
            }
            if (machine->is_break || machine->is_continue) {
                puts("Invalid control flow statement with no match "
                     "found.");
                exit(EXIT_FAILURE);
            }
        }
        machine->current_scope = scope_delete(machine->current_scope);
        return get_strid_empty();
    }

    default:
        puts("unimplemented");
        exit(EXIT_FAILURE);
    }
}

void interpret_node(Machine *machine, Node *ast) {
    switch (ast->type) {
    case NODE_PROGRAM:
        for (int i = 0; i < ast->program.program_len; i++) {
            interpret_node(machine, &ast->program.nodes[i]);
            if (machine->is_break || machine->is_continue ||
                machine->return_pending) {
                puts("Invalid control flow expression at top level "
                     "scope.");
                exit(EXIT_FAILURE);
            }
        }
        break;

    case NODE_TOP_EXPR:
        eval_expr(machine, ast->top_expr);
        break;

    case NODE_IF: {
        StrId expr = eval_expr(machine, ast->if_statement.expr);
        if (expr == get_strid_true()) {
            machine->current_scope =
                scope_make_child(machine->current_scope, true);
            for (size_t i = 0; i < ast->if_statement.body_node_count; i++) {
                interpret_node(machine, &ast->if_statement.body[i]);
                if (machine->is_break || machine->is_continue ||
                    machine->return_pending) {
                    machine->current_scope =
                        scope_delete(machine->current_scope);
                    return;
                }
            }
            machine->current_scope = scope_delete(machine->current_scope);
        } else {
            for (size_t i = 0; i < ast->if_statement.num_elifs; i++) {
                Node *elif = &ast->if_statement.elifs[i];
                StrId elif_expr = eval_expr(machine, elif->elif_statement.expr);
                if (elif_expr == get_strid_true()) {
                    machine->current_scope =
                        scope_make_child(machine->current_scope, true);
                    for (size_t i = 0; i < elif->elif_statement.body_node_count;
                         i++) {
                        interpret_node(machine, &elif->elif_statement.body[i]);
                        if (machine->is_break || machine->is_continue ||
                            machine->return_pending) {
                            machine->current_scope =
                                scope_delete(machine->current_scope);
                            return;
                        }
                    }
                    machine->current_scope =
                        scope_delete(machine->current_scope);
                    break;
                }
            }
        }
        break;
    }

    case NODE_WHILE: {
        machine->current_scope = scope_make_child(machine->current_scope, true);
        while (eval_expr(machine, ast->while_statement.expr) ==
               get_strid_true()) {
            for (size_t i = 0; i < ast->while_statement.body_node_count; i++) {
                interpret_node(machine, &ast->while_statement.body[i]);
                if (machine->is_break || machine->return_pending) {
                    machine->is_break = false;
                    goto break_sl_while;
                }
                if (machine->is_continue) {
                    machine->is_continue = false;
                    break;
                }
            }
            scope_clear(machine->current_scope);
        }
    break_sl_while:
        machine->current_scope = scope_delete(machine->current_scope);
        break;
    }

    case NODE_FUNCDECL: {
        machine_add_func(machine, ast->func_decl.name, ast);
        break;
    }

    case NODE_RETURN: {
		if (machine->current_scope == NULL) {
			puts("Return statement is not allowed on the global scope.");
			exit(EXIT_FAILURE);
		}

        if (ast->return_statement != NULL) {
            machine->return_pending = eval_expr(machine, ast->return_statement);
        } else {
            machine->return_pending = get_strid_empty();
        }
        break;
    }

    case NODE_BREAK: {
        machine->is_break = true;
        break;
    }

    case NODE_CONTINUE: {
        machine->is_continue = true;
        break;
    }

    default:
        puts("unimplemented");
        exit(EXIT_FAILURE);
    }
}

void interpret(Node *ast) {
    Scope global_scope = scope_make();
    Machine machine = (Machine){
        .global_scope = global_scope,
        .current_scope = NULL,
        .return_pending = STRID_NULL,
        .is_break = false,
        .is_continue = false,
    };
    machine_import_builtin_lib(&machine, BUILTIN_LIB_ESSENTIAL);
    interpret_node(&machine, ast);
}
