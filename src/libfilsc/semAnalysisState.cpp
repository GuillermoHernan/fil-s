#include "pch.h"
#include "semAnalysisState.h"
#include "symbolScope.h"

Ref<AstNode> SemAnalysisState::parent(unsigned index)const
{
	if (index < m_parents.size())
		return *(m_parents.rbegin()+index);
	else
		return Ref<AstNode>();
}

void SemAnalysisState::pushParent(Ref<AstNode> node)
{
	m_parents.push_back(node);
}

Ref<AstNode> SemAnalysisState::popParent()
{
	assert(!m_parents.empty());

	auto result = m_parents.back();
	m_parents.pop_back();

	return result;
}

/// <summary>
/// Finds the first parent for which the predicate 'pred' evaluates to 'true'
/// </summary>
/// <param name=""></param>
/// <returns>The first parent node which fulfills the predicate or null</returns>
Ref<AstNode> SemAnalysisState::findParent(std::function<bool(Ref<AstNode>)> pred)const
{
	for (auto it = m_parents.rbegin(); it != m_parents.rend(); ++it)
	{
		if (pred(*it))
			return *it;
	}

	return Ref<AstNode>();
}



SemAnalysisState::SemAnalysisState()
	:rootScope (SymbolScope::create(Ref<SymbolScope>()))
{
}
