#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"

int is_op(TokenType type) {
  switch (type) {
  case TOK_EQUALS:
  case TOK_PLUS:
  case TOK_AMPER:
  case TOK_BANGAMPER:
  case TOK_DOUBLEEQ:
  case TOK_BANGEQ:
    return 1;
  default:
    return 0;
  }
}

OpType to_optype(TokenType type) {
  switch (type) {
  case TOK_EQUALS:
    return OP_SET;
  case TOK_PLUS:
    return OP_CONCAT;
  case TOK_AMPER:
    return OP_STR_AND;
  case TOK_BANGAMPER:
    return OP_STR_NAND;
  case TOK_DOUBLEEQ:
    return OP_EQ;
  case TOK_BANGEQ:
    return OP_NOT_EQ;
  default:
    puts("No matching optype\n");
    exit(1);
  }
}

typedef struct {
  size_t row, col;
  size_t data_i;
  size_t data_len;
  const char *data;

  Node *ast;
} ParseState;

ParseState make_parser(const char *data, size_t len) {
  ParseState ps = (ParseState){
      .row = 0, .col = 0, .data_len = len, .data_i = 0, .data = data};
  return ps;
}

//---------------------
// Toker start
//---------------------

int isidentch(char ch);
char bump(ParseState *state);
void tok_ident_or_keyword(ParseState *state, TokenType *out_type);
char *tok_str(ParseState *state, size_t *out_len);
Token tok_next(ParseState *state);
TokenType tok_peek_type(ParseState *state);

int isidentch(char ch) { return ch == '_' || isalpha(ch) || isdigit(ch); }
char peek(ParseState *state) {
  if (state->data_i >= state->data_len) {
    return EOF;
  }
  return state->data[state->data_i];
}

char bump(ParseState *state) {
  if (state->data_i >= state->data_len) {
    return EOF;
  }
  const char byte = state->data[state->data_i];

  state->data_i += 1;
  if (byte == '\n') {
    state->row += 1;
    state->col = 0;
  } else {
    state->col += 1;
  }

  return byte;
}

void tok_ident_or_keyword(ParseState *state, TokenType *out_type) {
  static size_t keyword_lengths[] = {
      strlen("function"),
      strlen("if"),
      strlen("while"),
  };
  static size_t keywords_lengths_len = sizeof(keyword_lengths) / sizeof(size_t);

  // NOTE: make sure these are in the same order as in the enum
  static char *keywords[] = {
      "function",
      "if",
      "while",
  };

  const char *start = state->data + state->data_i - 1;
  while (isidentch(peek(state))) {
    bump(state);
  }
  *out_type = TOK_IDENT;

  size_t len = (state->data + state->data_i) - start;
  for (int i = 0; i < keywords_lengths_len; i++) {
    if (keyword_lengths[i] == len && strncmp(start, keywords[i], len) == 0) {
      // The reason i add 'i' is because the enum variant is calculated based on
      // the keyword index. This works because TOK_FUNCTION should always be the
      // first keyword defined in the TokeType enum.
      *out_type = TOK_FUNCTION + i;
    }
  }
}

char *tok_str(ParseState *state, size_t *out_len) {
  size_t capacity = 50;
  char *buf = malloc(capacity);
  if (buf == NULL) {
    puts("Error allocating string token");
    exit(1);
  }

  size_t idx = 0;
  char next;
  while ((next = bump(state)) != '"') {
    if (next == EOF) {
      puts("Tried to parse string but ran into EOF\n");
      exit(1);
    }
    if (idx == capacity) {
      capacity *= 2;
      buf = realloc(buf, capacity);
    }
    buf[idx] = next;
    idx += 1;
  }
  buf = realloc(buf, idx + 1);
  // null terminate it lol
  buf[idx] = '\0';

  // return values
  *out_len = idx;
  return buf;
}

Token tok_next(ParseState *state) {
  const size_t row = state->row;
  const size_t col = state->col;
  const size_t start = state->data_i;

  // where the thing starts
  const char *ptr = state->data + state->data_i;
  TokenType type = TOK_UNKNOWN;

  char byte = bump(state);
  while (isspace(byte) && byte != '\n') {
    byte = bump(state);
  }

  if (byte == '"') {
    // String!!!
    size_t len = 0;
    ptr = tok_str(state, &len);

    printf("%lu, %lu\n", row, col);
    return (Token){
        .slice = (Slice){.ptr = ptr, .len = len},
        .row = row,
        .col = col,
        .type = TOK_STR,
    };

  } else if (isidentch(byte)) {
    TokenType out;
    tok_ident_or_keyword(state, &out);
    type = out;
  } else {
    TokenType next_type;
    switch (byte) {
    case EOF:
      type = TOK_EOF;
      break;
    case '\n':
      type = TOK_NEWLINE;
      break;
    case '(':
      type = TOK_OPAREN;
      break;
    case ')':
      type = TOK_CPAREN;
      break;
    case '{':
      type = TOK_OCURLY;
      break;
    case '}':
      type = TOK_CCURLY;
      break;
    case '=':
      type = TOK_EQUALS;
      next_type = tok_peek_type(state);
      if (next_type == TOK_EQUALS) {
        type = TOK_DOUBLEEQ;
        tok_next(state);
      }
      break;
    case '+':
      type = TOK_PLUS;
      break;
    case '!':
      type = TOK_BANG;
      next_type = tok_peek_type(state);
      if (next_type == TOK_AMPER) {
        type = TOK_BANGAMPER;
        tok_next(state);
      } else if (next_type == TOK_EQUALS) {
        type = TOK_BANGEQ;
        tok_next(state);
      }
      break;
    case '&':
      type = TOK_AMPER;
      break;
    case ',':
      type = TOK_COMMA;
      break;
    }
  }
  printf("%lu, %lu\n", row, col);
  return (Token){
      .slice = (Slice){.ptr = ptr, .len = state->data_i - start},
      .row = row,
      .col = col,
      .type = type,
  };
}

TokenType tok_peek_type(ParseState *state) {
  size_t row = state->row;
  size_t col = state->col;
  size_t data_i = state->data_i;

  Token token = tok_next(state);

  if (token.type == TOK_STR) {
    free((void *)token.slice.ptr);
  }

  state->row = row;
  state->col = col;
  state->data_i = data_i;

  return token.type;
}

//---------------------------
// Parser start
//---------------------------
//
void print_expr(Expr *expr);
void print_ast(Node *node);
int expect(ParseState *state, TokenType type);
void infix_binding_power(OpType op, char *out_leftbp, char *out_rightbp);
Expr *parse_arg_list(ParseState *state, size_t *out_len);
Expr *parse_expr_single(ParseState *state);
Expr *parse_expr_bp(ParseState *state, int min_bp);
Expr *parse_expr(ParseState *state);
Node *parse_var_decl(ParseState *state);
Node *parse(ParseState *state);

void print_expr(Expr *expr) {
  switch (expr->type) {
  case EXPR_IDENT:
    printf("Ident: %.*s", (int)expr->ident.len, expr->ident.ptr);
    break;
  case EXPR_VALUE:
    printf("Value: %.*s", (int)expr->value.len, expr->value.ptr);
    break;
  case EXPR_TERM:
    printf("Term: { ");
    printf("Op: %d, ", expr->term.op);
    print_expr(expr->term.left);
    printf(", ");
    print_expr(expr->term.right);
    printf(" }");
    break;
  case EXPR_CALL:
    printf("Call: { ");
    printf("Name: %.*s, ", (int)expr->call.name.len, expr->call.name.ptr);
    printf("Args: { ");
    for (int i = 0; i < expr->call.num_args; i++) {
      print_expr(&expr->call.args[i]);
      if (i < expr->call.num_args - 1) {
        printf(", ");
      }
    }
    printf("} }");
    break;
  }
}

void print_ast(Node *node) {
  switch (node->type) {
  case NODE_PROGRAM:
    printf("Program { ");
    for (int i = 0; i < node->program.program_len; i++) {
      print_ast(&node->program.nodes[i]);
      if (i < node->program.program_len - 1) {
        printf(", ");
      }
    }
    printf("}");
    break;
	
  case NODE_TOP_EXPR:
	printf("Expr { ");
	print_expr(node->top_expr);
	printf("}");
	break;
  default:
    printf("nuh uh\n");
    exit(1);
  }
}

void infix_binding_power(OpType op, char *out_leftbp, char *out_rightbp) {
  char left;
  char right;
  switch (op) {
  case OP_CONCAT:
    left = 7;
    right = 8;
    break;
  case OP_EQ:
  case OP_NOT_EQ:
    left = 5;
    right = 6;
    break;
  case OP_BOOL_AND:
  case OP_BOOL_OR:
    left = 3;
    right = 4;
    break;
  case OP_SET:
    left = 1;
    right = 2;
	break;
  default:
    puts("TODO\n");
    exit(1);
  }
  *out_leftbp = left;
  *out_rightbp = right;
}

Node *alloc_node(NodeType type) {
  Node *node = malloc(sizeof(Node));
  if (node == NULL) {
    printf("Error allocating node of type %d\n", type);
    exit(1);
  }
  node->type = type;
  return node;
}

Expr *alloc_expr(ExprType type) {
  Expr *expr = malloc(sizeof(Expr));
  if (expr == NULL) {
    printf("Error allocating node of type %d\n", type);
    exit(1);
  }
  expr->type = type;
  return expr;
}

int expect(ParseState *state, TokenType type) {
  return tok_next(state).type == type;
}

Expr *parse_arg_list(ParseState *state, size_t *out_len) {
  if (expect(state, TOK_OPAREN) == 0) {
    puts("error, parse_arg_list");
    exit(1);
  }

  Expr *args = NULL;
  size_t len = 0;

  TokenType peek;
  while ((peek = tok_peek_type(state)) != TOK_CPAREN) {
    if (peek == TOK_EOF) {
      puts("error, parse_expr_lhs");
      exit(1);
    }
    Expr *expr = parse_expr(state);

    if (args == NULL) {
      args = malloc(sizeof(Expr));
      len = 1;
    } else {
      len += 1;
      args = realloc(args, len * sizeof(Expr));
    }
    // len can't be < 1.
    args[len - 1] = *expr;
    free(expr);

    if (tok_peek_type(state) == TOK_COMMA) {
      tok_next(state);
    }
  }

  printf("going past paren\n");
  if (expect(state, TOK_CPAREN) == 0) {

    puts("error, parse_param_list\n");
    exit(1);
  }

  *out_len = len;
  return args;
}

Expr *parse_expr_single(ParseState *state) {
  Token t = tok_next(state);
  Expr *expr;
  if (t.type == TOK_STR) {
    Expr *expr = alloc_expr(EXPR_VALUE);
    expr->value = t.slice;

    return expr;

  } else if (t.type == TOK_IDENT) {
    TokenType type = tok_peek_type(state);
    if (type == TOK_OPAREN) {
      printf("%d\n", type);
      size_t len;
      Expr *args = parse_arg_list(state, &len);

      Expr *expr = alloc_expr(EXPR_CALL);
      expr->call.args = args;
      expr->call.num_args = len;
      expr->call.name = t.slice;

      return expr;

    } else {
      Expr *expr = alloc_expr(EXPR_IDENT);
      expr->ident = t.slice;

      return expr;
    }
  }

	// TODO: make it so newline doesn't cause this
  puts("error, parse_expr_single\n");
  exit(1);
}

Expr *parse_expr_bp(ParseState *state, int min_bp) {

  Expr *lhs;
  TokenType temp = tok_peek_type(state);
  printf("%d\n", temp);
  if (is_op(temp) == 1) {
    printf("hi\n");
    tok_next(state);
    OpType op = to_optype(temp);
    char ignore, rightbp;
    infix_binding_power(op, &ignore, &rightbp);
    Expr *rhs = parse_expr_bp(state, rightbp);
    lhs = rhs;

  } else {
    lhs = parse_expr_single(state);
  }

  while (1) {
    TokenType next_type = tok_peek_type(state);
    OpType op;
    if (is_op(next_type) == 1) {
      op = to_optype(next_type);
    } else if (next_type == TOK_EOF || next_type == TOK_COMMA ||
               next_type == TOK_CPAREN || next_type == TOK_NEWLINE) {
      break;
    } else {
      puts("error, invalid operator in parse_expr_bp");
      exit(1);
    }

    char leftbp, rightbp;
    infix_binding_power(op, &leftbp, &rightbp);

    if (leftbp < min_bp) {
      break;
    }

    tok_next(state);
    Expr *rhs = parse_expr_bp(state, rightbp);

    Expr *expr = alloc_expr(EXPR_TERM);
    expr->term.left = lhs;
    expr->term.right = rhs;
    expr->term.op = op;

    lhs = expr;
  }
  return lhs;
}

Expr *parse_expr(ParseState *state) {
  Expr *expr = parse_expr_bp(state, 0);
  return expr;
}

Node *parse(ParseState *state) {
  state->ast = alloc_node(NODE_PROGRAM);
  Node *nodes = malloc(sizeof(Node));
  size_t nodes_len = 0;

  TokenType token_type;
  while ((token_type = tok_peek_type(state)) != TOK_EOF) {
    if (token_type == TOK_IDENT) {
      Expr *expr = parse_expr(state);
      Node *node = alloc_node(NODE_TOP_EXPR);
      node->top_expr = expr;

      nodes[nodes_len] = *node;
      free(node);
      nodes_len += 1;
      nodes = realloc(nodes, sizeof(Node) * (nodes_len + 1));
    } else {
		tok_next(state);
	}
  }
  state->ast->program.nodes = nodes;
  state->ast->program.program_len = nodes_len;

  return state->ast;
}

void test() {
  const char *mock_file =
      "x = test(\"asdf\" + \"mkmd\" + func(\"a\", \"b\")) + \"hai\"\n"
      "my_str_var = x == \"help\" != func() + \"hi\"\n";
  const size_t len = strlen(mock_file);
  ParseState ps = make_parser(mock_file, len);

  Node *ast = parse(&ps);

  print_ast(ast);
  puts("");
}
