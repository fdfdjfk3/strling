#ifndef PARSE_H
#define PARSE_H
#include "interner/interner.h"
#include "types.h"
#include <stdlib.h>

typedef enum {
    PARSE_GLOBAL,
    PARSE_BLOCK,
    PARSE_FUNC,
} ParseContext;

typedef enum {
    TOK_UNKNOWN = 0,
    TOK_EOF = 1,
    TOK_NEWLINE = 2,
    TOK_OPAREN = 3,
    TOK_CPAREN = 4,
    TOK_OCURLY = 5,
    TOK_CCURLY = 6,
    TOK_EQUALS = 7,
    TOK_PLUS = 8,
    TOK_MINUS = 9,
    TOK_DOUBLEEQ = 10,
    TOK_BANGEQ = 11,
    TOK_AMPER = 12,
    TOK_BANGAMPER = 13,
    TOK_COMMA = 14,
    TOK_IDENT = 15,
    TOK_FUNCTION = 16,
    TOK_IF = 17,
    TOK_WHILE = 18,
    TOK_ELIF = 19,
    TOK_RETURN = 20,
    TOK_REF = 21,
    TOK_BREAK = 22,
    TOK_CONTINUE = 23,
    TOK_AND = 24,
    TOK_OR = 25,
	TOK_IMPORT = 26,

    // The most important one.
    TOK_STR = 27,
} TokenType;

typedef struct {
    StrId str;
    size_t row, col, i;
    TokenType type;
} Token;

typedef enum {
    NODE_PROGRAM,
    // NODE_VARDECL,
    NODE_FUNCDECL,
    NODE_TOP_EXPR,
    NODE_IF,
    NODE_ELIF,
    NODE_WHILE,
    NODE_RETURN,
    NODE_BREAK,
    NODE_CONTINUE,
} NodeType;

typedef enum {
    OP_SET,
    OP_CONCAT,
    OP_EQ,
    OP_NOT_EQ,
    OP_INTERSECTION,
    OP_DIFFERENCE,
    OP_BOOL_AND,
    OP_BOOL_OR,
    OP_REMOVE_OCCUR,
} OpType;

typedef enum {
    EXPR_VALUE,
    EXPR_IDENT,
    EXPR_CALL,
    EXPR_TERM,
} ExprType;

typedef struct Expr {
    ExprType type;
    union {
        // String literal
        StrId value;

        // Identifier (variable name)
        StrId ident;

        // Function call with arguments
        struct {
            StrId name;
            struct Expr *args;
            char num_args;
        } call;

        // Binary operation
        struct {
            struct Expr *left;
            struct Expr *right;
            OpType op;
        } term;
    };
} Expr;

typedef struct Node {
    NodeType type;
    union {
        // *
        struct {
            struct Node *nodes;
            size_t program_len;
        } program;

        // <expr>
        Expr *top_expr;
        // function <name>(<param_names>[0], ...<param_count>) {
        //   <body>
        // }
        struct {
            StrId name;
            StrId *param_names;
            bool *param_is_ref;
            // max 255 params
            char param_count;

            struct Node *body;
            size_t body_node_count;
        } func_decl;

        // while <expr> {
        //   <body>
        // }
        struct {
            Expr *expr;
            struct Node *body;
            size_t body_node_count;
        } while_statement;

        // if <expr> {
        //   <body>
        // }
        struct {
            Expr *expr;
            struct Node *body;
            size_t body_node_count;

            struct Node *elifs;
            size_t num_elifs;
        } if_statement;

        // } elif <expr> {
        struct {
            Expr *expr;
            struct Node *body;
            size_t body_node_count;
        } elif_statement;

        // return <expr>
        Expr *return_statement;
    };
} Node;

void print_interner(Interner *i);
Node *test();
Node *parse_file(const char *filename);

#endif
