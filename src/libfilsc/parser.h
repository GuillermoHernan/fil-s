/// <summary>
/// Parses FIL-S code into an Abstract Sintax Tree (AST) structure.
/// </summary>

#pragma once

#include "parserResults.h"

class LexToken;

ExprResult parseFile(SourceFilePtr fileRef);
ExprResult parseScript(const char* script, SourceFilePtr fileRef);
ExprResult parseScript(LexToken token);
ExprResult parseStatement(LexToken token);

