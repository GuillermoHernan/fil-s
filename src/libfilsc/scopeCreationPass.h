/// <summary>
/// Pass of the semantic analysis phase in which the scope hierarchy is created
/// and assigned to the AST nodes.
/// </summary>

#pragma once

#include "ast.h"

class SemanticResult;
class SemAnalysisState;

SemanticResult scopeCreationPass(Ref<AstNode> node, SemAnalysisState& state);
