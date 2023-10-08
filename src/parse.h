#ifndef PARSE_H
#define PARSE_H
#include <stdlib.h>

void test();

typedef struct {
  const char *ptr;
  size_t len;
} Slice;

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
  TOK_BANG = 9,
  TOK_AMPER = 10,
  TOK_BANGAMPER = 11,
  TOK_DOUBLEEQ = 12,
  TOK_BANGEQ = 13,
  TOK_COMMA = 14,
  TOK_IDENT = 15,
  TOK_FUNCTION = 16,
  TOK_IF = 17,
  TOK_WHILE = 18,

  // The most important one.
  TOK_STR = 19,
} TokenType;

typedef struct {
  Slice slice;
  size_t row, col;
  TokenType type;
} Token;

typedef enum {
  NODE_PROGRAM,
  NODE_VARDECL,
  NODE_FUNCDECL,
  NODE_TOP_EXPR,
  NODE_IF,
  NODE_WHILE,
} NodeType;

typedef enum {
	OP_SET,
  OP_CONCAT,
  OP_EQ,
  OP_NOT_EQ,
  OP_STR_AND,
  OP_STR_NAND,
  OP_BOOL_AND,
  OP_BOOL_OR,
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
    Slice value;

	// Identifier (variable name)
    Slice ident;

	// Function call with arguments
    struct {
      Slice name;
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
      Slice name;
      Slice *param_names;
      // max 255 params
      char param_count;

      struct Node **body;
      size_t body_node_count;
    } func_decl;

    // while <expr> {
    //   <body>
    // }
    struct {
      Expr *expr;
      struct Node **body;
      size_t body_node_count;
    } while_statement;

    // if <expr> {
    //   <body>
    // }
    struct {
      Expr *expr;
      struct Node **body;
      size_t body_node_count;
    } if_statement;
  };
} Node;
#endif
