#ifndef PARSER_H
#define PARSER_H

#include "../ast/ast.h"
#include "../lexer/lexer.h"

ASTNode *parser_logique(token **current);
ASTNode *parser(token *token);

#endif // PARSER_H
