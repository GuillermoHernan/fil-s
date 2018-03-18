#include "pch.h"
#include "semAnalysisState.h"
#include "symbolScope.h"

Ref<AstNode> SemAnalysisState::parent(unsigned index)const
{
    if (index < m_parents.size())
        return *(m_parents.rbegin() + index);
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
    :rootScope(SymbolScope::create(Ref<SymbolScope>()))
{
}

/// <summary>
/// Gets the scope assigned to a node.
/// </summary>
/// <param name="node"></param>
/// <returns></returns>
Ref<SymbolScope> SemAnalysisState::getScope(const AstNode* node)const
{
    auto it = m_scopesMap.find(node);

    assert(it != m_scopesMap.end());
    return it->second;
}

/// <summary>
/// Gets the scope assigned to a node.
/// </summary>
/// <param name="node"></param>
/// <returns></returns>
Ref<SymbolScope> SemAnalysisState::getScope(Ref<AstNode> node)const
{
    return getScope(node.getPointer());
}

/// <summary>
/// Assigns a scope to a node.
/// </summary>
/// <param name="node"></param>
/// <param name=""></param>
void SemAnalysisState::setScope(Ref<AstNode> node, Ref<SymbolScope> scope)
{
    m_scopesMap[node.getPointer()] = scope;
}
