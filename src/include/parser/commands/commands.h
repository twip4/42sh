#ifndef COMMANDS_H
#define COMMANDS_H

#include "../parser.h"
#include "../../ast/ast.h"
#include "../../lexer/lexer.h"
#include "../../garbage/garbage.h"

ASTNode *parser_if(token **current);
ASTNode *parser_for(token **current);
ASTNode *parser_while(token **current);

#endif // COMMANDS_H