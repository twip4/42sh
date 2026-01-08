#ifndef PARSER_H
#define PARSER_H

#include "../ast/ast.h"
#include "../lexer/lexer.h"

ASTNode *parser(token *token);

#endif // PARSER_H
