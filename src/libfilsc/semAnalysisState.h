#pragma once

#include "ast.h"
#include "SymbolScope.h"

/// <summary>
/// Holds the current state of the semantic analyzer.
/// Keeps it between passes
/// </summary>
class SemAnalysisState
{
public:
	Ref<SymbolScope>	rootScope;

	Ref<AstNode>		parent()const;

	void				pushParent(Ref<AstNode> node);
	Ref<AstNode>		popParent();

private:
	std::vector<Ref<AstNode>>	m_parents;
};
