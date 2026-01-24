#ifndef COMMANDS_H
#define COMMANDS_H

#include "../../ast/ast.h"
#include "../../garbage/garbage.h"
#include "../../lexer/lexer.h"
#include "../parser.h"

ASTNode *parser_if(token **current);
ASTNode *parser_for(token **current);
ASTNode *parser_while(token **current);

#endif // COMMANDS_H

