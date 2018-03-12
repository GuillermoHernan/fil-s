/// <summary>
/// Parses FIL-S code into an Abstract Sintax Tree (AST) structure.
/// </summary>

#pragma once

#include "parserResults.h"

class LexToken;

ExprResult parseFile(const std::string& path);
ExprResult parseScript(const char* script);
ExprResult parseScript(LexToken token);
ExprResult parseStatement(LexToken token);

