/// <summary>
/// Semantic analizer.
/// Checks and transforms the AST in order to provide an AST ready for code generation.
/// </summary>

#pragma once

#include "ast.h"
#include "compileError.h"

class SemanticResult;

SemanticResult semanticAnalysis(Ref<AstNode> node);

/// <summary>
/// Object which yields the semantic analyzer.
/// </summary>
class SemanticResult
{
public:
	SemanticResult(Ref<AstNode> node) : ast(node)
	{}

	SemanticResult(const CompileError& error)
	{
		errors.push_back(error);
	}


	SemanticResult(const std::vector<CompileError>& errors_)
	{
		errors = errors_;
	}

	bool ok()const
	{
		return errors.empty();
	}

	SemanticResult combineWith(const SemanticResult& r2)const;

	Ref<AstNode>				ast;
	std::vector<CompileError>	errors;
};
