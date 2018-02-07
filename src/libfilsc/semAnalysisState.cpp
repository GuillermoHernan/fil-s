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


SemAnalysisState::SemAnalysisState()
	:rootScope (SymbolScope::create(Ref<SymbolScope>()))
{
}
