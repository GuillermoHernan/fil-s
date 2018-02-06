#include "pch.h"
#include "semAnalysisState.h"

Ref<AstNode> SemAnalysisState::parent()const
{
	if (!m_parents.empty())
		return m_parents.back();
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
