#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "globals.h"

int is_op(TokenType type) {
  switch (type) {
  case TOK_EQUALS:
  case TOK_PLUS:
  case TOK_DOUBLEEQ:
  case TOK_BANGEQ:
  case TOK_AMPER:
  case TOK_BANGAMPER:
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
  case TOK_DOUBLEEQ:
    return OP_EQ;
  case TOK_BANGEQ:
    return OP_NOT_EQ;
  case TOK_AMPER:
	return OP_INTERSECTION;
  case TOK_BANGAMPER:
	return OP_DIFFERENCE;
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
  ParseState ps = (ParseState){.row = 0,
                               .col = 0,
                               .data_len = len,
                               .data_i = 0,
                               .data = data,
                               };
  return ps;
}

//---------------------
// Toker start
//---------------------

int isidentch(char ch);
char bump(ParseState *state);
TokenType tok_ident_or_keyword(ParseState *state);
StrId tok_str(ParseState *state);
Token tok_next(ParseState *state);
TokenType tok_peek_type(ParseState *state);

int isidentch(char ch) { return ch == '_' || isalpha(ch) || isdigit(ch); }
char peek(ParseState *state) {
  if (state->data_i >= state->data_len) {
    return EOF;
  }
  return state->data[state->data_i];
}

char peekch(ParseState *state) { return state->data[state->data_i]; }

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

TokenType tok_ident_or_keyword(ParseState *state) {
  // NOTE: make sure these are in the same order as in the enum
  static char *keywords[] = {
      "function", "if", "while", "elif", "else", "return",
  };
  static size_t keyword_lengths[] = {
      strlen("function"), strlen("if"),   strlen("while"),
      strlen("elif"),     strlen("else"), strlen("return"),
  };
  static size_t num_keywords = sizeof(keyword_lengths) / sizeof(size_t);

  const char *start = state->data + state->data_i - 1;
  while (isidentch(peek(state))) {
    bump(state);
  }

  size_t len = (state->data + state->data_i) - start;
  for (int i = 0; i < num_keywords; i++) {
    if (keyword_lengths[i] == len && strncmp(start, keywords[i], len) == 0) {
      // The reason i add 'i' is because the enum variant is calculated based on
      // the keyword index. This works because TOK_FUNCTION should always be the
      // first keyword defined in the TokeType enum.
      return TOK_FUNCTION + i;
    }
  }
  return TOK_IDENT;
}

StrId tok_str(ParseState *state) {
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
	if (next == '\\') {
		next = bump(state);
		if (next == 'n') {
			buf[idx] = '\n';
		} else {
			puts("escape seq not implemented");
			exit(1);
		}
	} else {
		buf[idx] = next;
	}
    idx += 1;
  }

  buf = realloc(buf, idx);
  StrId ret = g_interner_intern_noalloc(buf, idx);

  return ret;
}

Token tok_next(ParseState *state) {
  char peek = peekch(state);
  while (isspace(peek) && peek != '\n') {
    bump(state);
    peek = peekch(state);
  }

  size_t row = state->row;
  size_t col = state->col;
  size_t start = state->data_i;

  // where the thing starts
  const char *ptr = state->data + state->data_i;
  TokenType type = TOK_UNKNOWN;

  char byte = bump(state);

  if (byte == '"') {
    // String!!!
    StrId str = tok_str(state);

    // printf("%lu, %lu\n", row, col);
    return (Token){
        .str = str,
        .row = row,
        .col = col,
        .type = TOK_STR,
    };

  } else if (isidentch(byte)) {
    type = tok_ident_or_keyword(state);
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
    case ',':
      type = TOK_COMMA;
      break;
    case '!':
      next_type = tok_peek_type(state);
      if (next_type == TOK_EQUALS) {
        type = TOK_BANGEQ;
        tok_next(state);
        break;
      } else if (next_type == TOK_AMPER) {
        type = TOK_BANGAMPER;
        tok_next(state);
        break;
      }
      puts("invalid token");
      exit(1);
      break;
    case '&':
      type = TOK_AMPER;
      break;
    }
  }
  // printf("%lu, %lu\n", row, col);
  return (Token){
      .str = g_interner_intern(ptr, state->data_i - start),
      .row = row,
      .col = col,
      .type = type,
  };
}

TokenType tok_peek_type(ParseState *state) {
  ParseState copy = *state;

  char peek = peekch(&copy);
  while (isspace(peek) && peek != '\n') {
    bump(&copy);
    peek = peekch(&copy);
  }

  char next = bump(&copy);

  if (next == '"') {
    return TOK_STR;
  } else if (isidentch(next)) {
    return tok_ident_or_keyword(&copy);
  } else {
    switch (next) {
    case EOF:
      return TOK_EOF;
    case '\n':
      return TOK_NEWLINE;
    case '(':
      return TOK_OPAREN;
    case ')':
      return TOK_CPAREN;
    case '{':
      return TOK_OCURLY;
    case '}':
      return TOK_CCURLY;
    case '=':
      if (tok_peek_type(&copy) == TOK_EQUALS) {
        return TOK_DOUBLEEQ;
      }
      return TOK_EQUALS;
    case '+':
      return TOK_PLUS;
    case ',':
      return TOK_COMMA;
    case '!': {
      TokenType peek = tok_peek_type(&copy);
      if (peek == TOK_EQUALS) {
        return TOK_BANGEQ;
      } else if (peek == TOK_AMPER) {
        return TOK_BANGAMPER;
      }
    }
    case '&':
      return TOK_AMPER;
    default:
      break;
    }
  }
  return TOK_UNKNOWN;
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
Node *parse_if_statement(ParseState *state);
Node *parse_while_statement(ParseState *state);
Node *parse_func_decl(ParseState *state);
Node *parse_body(ParseState *state, size_t *out_len, TokenType sentinel);
void parse(ParseState *state);

void print_expr(Expr *expr) {
  switch (expr->type) {
  case EXPR_IDENT:
    printf("Ident: %.*s", (int)expr->ident->len, expr->ident->ptr);
    break;
  case EXPR_VALUE:
    printf("Value: %.*s", (int)expr->value->len, expr->value->ptr);
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
    printf("Name: %.*s, ", (int)expr->call.name->len, expr->call.name->ptr);
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
    printf("Program: { ");
    for (int i = 0; i < node->program.program_len; i++) {
      print_ast(&node->program.nodes[i]);
      if (i < node->program.program_len - 1) {
        printf(", ");
      }
    }
    printf("}");
    break;

  case NODE_TOP_EXPR:
    printf("Expr: { ");
    print_expr(node->top_expr);
    printf("}");
    break;
  case NODE_IF:
    printf("IfStatement: { ");
    printf("Cond: { ");
    print_expr(node->if_statement.expr);
    printf("}, Body: { ");
    for (int i = 0; i < node->if_statement.body_node_count; i++) {
      print_ast(&node->if_statement.body[i]);
      if (i < node->if_statement.body_node_count - 1) {
        printf(", ");
      }
    }
    printf("}, Elifs: { ");
    for (int i = 0; i < node->if_statement.num_elifs; i++) {
      print_ast(&node->if_statement.elifs[i]);
      if (i < node->if_statement.num_elifs - 1) {
        printf(", ");
      }
    }
    break;
  case NODE_ELIF:
    printf("Elif: { ");
    printf("Cond: { ");
    print_expr(node->elif_statement.expr);
    printf("}, Body: { ");
    for (int i = 0; i < node->elif_statement.body_node_count; i++) {
      print_ast(&node->elif_statement.body[i]);
      if (i < node->elif_statement.body_node_count - 1) {
        printf(", ");
      }
    }
    printf("}}");
    break;
  case NODE_WHILE:
    printf("WhileStatement: { ");
    printf("Cond: { ");
    print_expr(node->while_statement.expr);
    printf("}, Body: { ");
    for (int i = 0; i < node->while_statement.body_node_count; i++) {
      print_ast(&node->while_statement.body[i]);
      if (i < node->while_statement.body_node_count - 1) {
        printf(", ");
      }
    }
    break;
  case NODE_FUNCDECL:
    printf("FunctionDecl: { ");
    printf("Name: %.*s, ", (int)node->func_decl.name->len,
           node->func_decl.name->ptr);
    printf("Params: { ");
    for (int i = 0; i < node->func_decl.param_count; i++) {
      printf("%.*s", (int)node->func_decl.param_names[i]->len,
             node->func_decl.param_names[i]->ptr);
      if (i < node->func_decl.param_count - 1) {
        printf(", ");
      }
    }
    printf("}, ");
    printf("Body: { ");
    for (int i = 0; i < node->func_decl.body_node_count; i++) {
      print_ast(&node->func_decl.body[i]);
      if (i < node->func_decl.body_node_count - 1) {
        printf(", ");
      }
    }
    printf("}}");
    break;
  default:
    puts("nuh uh");
    exit(1);
  }
}

void infix_binding_power(OpType op, char *out_leftbp, char *out_rightbp) {
  char left;
  char right;
  switch (op) {
  case OP_CONCAT:
    left = 9;
    right = 10;
    break;
  case OP_DIFFERENCE:
  case OP_INTERSECTION:
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
    expr->value = t.str;

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
      expr->call.name = t.str;

      return expr;

    } else {
      Expr *expr = alloc_expr(EXPR_IDENT);
      expr->ident = t.str;

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
  } else if (temp == TOK_OPAREN) {
    tok_next(state);
    lhs = parse_expr_bp(state, 0);
    if (expect(state, TOK_CPAREN) == 0) {
      puts("NO CLOSING PAREN fdlksfa jaslkfjdslfsf");
      exit(1);
    }
  } else {
    lhs = parse_expr_single(state);
  }

  while (1) {
    TokenType next_type = tok_peek_type(state);
    OpType op;
    if (is_op(next_type) == 1) {
      op = to_optype(next_type);
    } else if (next_type == TOK_EOF || next_type == TOK_COMMA ||
               next_type == TOK_CPAREN || next_type == TOK_NEWLINE ||
               next_type == TOK_OCURLY || next_type == TOK_CCURLY) {
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

Node *parse_if_statement(ParseState *state) {
  if (expect(state, TOK_IF) == 0) {
    puts("error, parse_if_statement");
    exit(1);
  }

  Expr *expr = parse_expr(state);
  if (expect(state, TOK_OCURLY) == 0) {
    puts("error, parse_if_statement");
    exit(1);
  }

  size_t len;
  Node *body = parse_body(state, &len, TOK_CCURLY);
  if (expect(state, TOK_CCURLY) == 0) {
    puts("error, parse_if_statement");
    exit(1);
  }

  Node *statement = alloc_node(NODE_IF);
  statement->if_statement.expr = expr;
  statement->if_statement.body = body;
  statement->if_statement.body_node_count = len;

  TokenType next;

  Node *elifs = NULL;
  size_t num_elifs = 0;

  while ((next = tok_peek_type(state)) == TOK_ELIF) {
    tok_next(state);
    Expr *expr = parse_expr(state);
    if (expect(state, TOK_OCURLY) == 0) {
      puts("error, parse_if_statement");
      exit(1);
    }

    size_t len;
    Node *body = parse_body(state, &len, TOK_CCURLY);

    num_elifs += 1;
    elifs = realloc(elifs, sizeof(Node) * num_elifs);
    elifs[num_elifs - 1].type = NODE_ELIF;
    elifs[num_elifs - 1].elif_statement.expr = expr;
    elifs[num_elifs - 1].elif_statement.body = body;
    elifs[num_elifs - 1].elif_statement.body_node_count = len;

    if (expect(state, TOK_CCURLY) == 0) {
      puts("error, parse_if_statement");
      exit(1);
    }
  }
  statement->if_statement.elifs = elifs;
  statement->if_statement.num_elifs = num_elifs;
  return statement;
}

Node *parse_while_statement(ParseState *state) {
  if (expect(state, TOK_WHILE) == 0) {
    puts("error, parse_while_statement");
    exit(1);
  }

  Expr *expr = parse_expr(state);
  if (expect(state, TOK_OCURLY) == 0) {
    puts("error, parse_while_statement");
    exit(1);
  }

  size_t len;
  Node *body = parse_body(state, &len, TOK_CCURLY);
  if (expect(state, TOK_CCURLY) == 0) {
    puts("error, parse_while_statement");
    exit(1);
  }

  Node *statement = alloc_node(NODE_WHILE);
  statement->while_statement.expr = expr;
  statement->while_statement.body = body;
  statement->while_statement.body_node_count = len;
  return statement;
}

Node *parse_func_decl(ParseState *state) {
  if (expect(state, TOK_FUNCTION) == 0) {
    puts("error, parse_func_decl");
    exit(1);
  }

  Token name = tok_next(state);
  if (name.type != TOK_IDENT) {
    puts("grrr u cant define a function with a non ident");
    exit(1);
  }

  // parse parameter list
  if (expect(state, TOK_OPAREN) == 0) {
    puts("error, parse_func_decl");
    exit(1);
  }

  StrId *params = NULL;
  size_t num_params = 0;
  Token temp;
  while ((temp = tok_next(state)).type != TOK_CPAREN) {
    printf("%d\n", temp.type);
    if (temp.type == TOK_IDENT) {
      num_params += 1;
      params = realloc(params, sizeof(StrId) * num_params);
      params[num_params - 1] = temp.str;

      Token next = tok_next(state);
      if (next.type == TOK_COMMA) {
        continue;
      }
      if (next.type == TOK_CPAREN) {
        break;
      } else {
        puts("error, invalid token found in parameter list");
        exit(1);
      }
    }
    puts("error, invalid token found in parameter list fkdjf");
    exit(1);
  }

  if (expect(state, TOK_OCURLY) == 0) {
    puts("error, parse_func_decl");
    exit(1);
  }

  size_t len;
  Node *body = parse_body(state, &len, TOK_CCURLY);

  if (expect(state, TOK_CCURLY) == 0) {
    puts("error, parse_func_decl");
    exit(1);
  }

  Node *decl = alloc_node(NODE_FUNCDECL);
  decl->func_decl.body_node_count = len;
  decl->func_decl.body = body;
  decl->func_decl.name = name.str;
  decl->func_decl.param_names = params;
  decl->func_decl.param_count = num_params;

  return decl;
}

Node *parse_body(ParseState *state, size_t *out_len, TokenType sentinel) {
  Node *nodes = malloc(sizeof(Node));
  size_t nodes_len = 0;

  TokenType token_type;
  // TODO: clean up this loop, it was very hastily put together.
  while ((token_type = tok_peek_type(state)) != sentinel) {
    if (token_type == TOK_IDENT) {
      Expr *expr = parse_expr(state);
      Node *node = alloc_node(NODE_TOP_EXPR);
      node->top_expr = expr;

      nodes[nodes_len] = *node;
      free(node);
      nodes_len += 1;
      nodes = realloc(nodes, sizeof(Node) * (nodes_len + 1));
    } else if (token_type == TOK_IF) {
      Node *node = parse_if_statement(state);

      nodes[nodes_len] = *node;
      free(node);
      nodes_len += 1;
      nodes = realloc(nodes, sizeof(Node) * (nodes_len + 1));
    } else if (token_type == TOK_WHILE) {
      Node *node = parse_while_statement(state);

      nodes[nodes_len] = *node;
      free(node);
      nodes_len += 1;
      nodes = realloc(nodes, sizeof(Node) * (nodes_len + 1));
    } else if (token_type == TOK_FUNCTION) {
      Node *node = parse_func_decl(state);

      nodes[nodes_len] = *node;
      free(node);
      nodes_len += 1;
      nodes = realloc(nodes, sizeof(Node) * (nodes_len + 1));
    } else if (token_type == TOK_RETURN && sentinel == TOK_CCURLY) {
      Node return_statement = (Node){.type = NODE_RETURN};
      tok_next(state);
      Expr *expr = parse_expr(state);
      return_statement.return_statement = expr;
      nodes[nodes_len] = return_statement;
      nodes_len += 1;
      nodes = realloc(nodes, sizeof(Node) * (nodes_len + 1));

    } else if (token_type == TOK_EOF) {
      break;
    } else {
      tok_next(state);
    }
  }

  *out_len = nodes_len;
  return nodes;
}

void parse(ParseState *state) {
  state->ast = alloc_node(NODE_PROGRAM);
  size_t len;
  Node *nodes = parse_body(state, &len, TOK_EOF);
  state->ast->program.program_len = len;
  state->ast->program.nodes = nodes;
}

void print_interner(Interner *i) {
  StrTable *table = &i->table;
  for (int i = 0; i < table->max_entries; i++) {
    if (table->entries[i] != NULL) {
      Slice str = *table->entries[i];
      printf("%d: ", i);
      fwrite(str.ptr, str.len, 1, stdout);
      puts("");
    }
  }
}

Node *test() {
  const char *mock_file = "print((\"hello\" & \"HEll\") + \"\n\")\n";
  const size_t len = strlen(mock_file);
  ParseState ps = make_parser(mock_file, len);

  parse(&ps);

  print_ast(ps.ast);
  puts("");
  print_interner(get_global_interner());
  puts("");
  return ps.ast;
}
