#include "parse.h"
#include "scope.h"

typedef struct {
    Scope global_scope;
    Scope *current_scope;
    StrId return_pending;
    int is_break;
    int is_continue;
} Machine;

typedef enum {
    I_GLOBAL,
    I_FUNC,
    I_BLOCK,
} IContext;

typedef enum {
    BUILTIN_LIB_ESSENTIAL,
} BuiltinLib;

void interpret(Node *ast);
void interpret_node(Machine *machine, Node *ast);
StrId interpret_node_in_func(Machine *machine, Node *node);
void interpret_node_in_block(Machine *machine, Node *node);
