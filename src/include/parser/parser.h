#ifndef PARSER_H
#define PARSER_H

#include "../ast/ast.h"
#include "../lexer/lexer.h"

ASTNode *parser_chain_until(token **current, const char *stop);
ASTNode *parser_chain(token **current);
ASTNode *parser(token *token);

#endif // PARSER_H
