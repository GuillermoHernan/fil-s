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
    assert(m_scopesMap.count(node.getPointer()) == 0);
    m_scopesMap[node.getPointer()] = scope;
}

/// <summary>
/// Tries to registers a new unnamed type (an unnamed tuple).
/// Only registers it if no equivalent tuple has been registered.
/// </summary>
/// <param name="tupleType"></param>
/// <returns>The tuple passed as parameter, or the previously registered tuple</returns>
Ref<AstNode> SemAnalysisState::registerUnnamedType(Ref<AstNode> tupleType)
{
    assert(tupleType->getType() == AST_TUPLE_DEF);
    auto result = m_unnamedTypes.insert(tupleType);

    if (result.second)
        return tupleType;
    else
        return *result.first;
}

/// <summary>
/// Gets the list of unnmaed types.
/// </summary>
/// <returns></returns>
AstNodeList SemAnalysisState::getUnnamedTypes()const
{
    return AstNodeList(m_unnamedTypes.begin(), m_unnamedTypes.end());
}


/// <summary>
/// Compares tuple types for sort them in a set or map.
/// </summary>
/// <param name="typeA"></param>
/// <param name="typeB"></param>
/// <returns></returns>
bool SemAnalysisState::CompareTupleTypes::operator()(Ref<AstNode> typeA, Ref<AstNode> typeB)
{
    assert(typeA->getType() == AST_TUPLE_DEF);
    assert(typeB->getType() == AST_TUPLE_DEF);

    if (typeA->childCount() != typeB->childCount())
        return typeA->childCount() < typeB->childCount();
    else
    {
        for (size_t i = 0; i < typeA->childCount(); ++i)
        {
            auto childAType = typeA->child(i)->getDataType();
            auto childBType = typeB->child(i)->getDataType();

            if (childAType != childBType)
                return childAType < childBType;
        }
    }

    //If they are equal, (typeA < typeB) is false.
    return false;
}
