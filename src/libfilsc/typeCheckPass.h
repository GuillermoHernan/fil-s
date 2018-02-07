/// <summary>
/// Compiler pass which checks that data types are correctly used.
/// </summary>
#pragma once

#include "ast.h"
class SemanticResult;
class SemAnalysisState;
class CompileError;

SemanticResult typeCheckPass(Ref<AstNode> node, SemAnalysisState& state);

CompileError typeExistsCheck(Ref<AstNode> node, SemAnalysisState& state);

bool isType(Ref<AstNode> node);
