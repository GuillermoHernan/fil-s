/// <summary>
/// Semantic analizer.
/// Checks and transforms the AST in order to provide an AST ready for code generation.
/// </summary>

#pragma once

#include "ast.h"
#include "operationResult.h"

typedef OperationResult<Ref<AstNode>> SemanticResult;

SemanticResult semanticAnalysis(Ref<AstNode> node);

SemanticResult semanticAnalysis(const AstStr2NodesMap& sources, const AstStr2NodesMap& modules);
