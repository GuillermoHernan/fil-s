#pragma once

#include "ast.h"
#include "SymbolScope.h"

#include <functional>

/// <summary>
/// Holds the current state of the semantic analyzer.
/// Keeps it between passes
/// </summary>
class SemAnalysisState
{
public:
	const Ref<SymbolScope>	rootScope;

	SemAnalysisState();

	Ref<AstNode>		parent(unsigned index = 0)const;

	void				pushParent(Ref<AstNode> node);
	Ref<AstNode>		popParent();

	Ref<AstNode>		findParent(std::function<bool(Ref<AstNode>)> pred)const;

private:
	std::vector<Ref<AstNode>>	m_parents;
};
