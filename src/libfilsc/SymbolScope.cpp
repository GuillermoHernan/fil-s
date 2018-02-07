/// <summary>
/// Implementation of symbol scope: stores the symbols used in the compiled program
/// in a hierarchical way.
/// </summary>

#include "pch.h"
#include "SymbolScope.h"

Ref<SymbolScope> SymbolScope::create(Ref<SymbolScope> parent)
{
	return refFromNew(new SymbolScope(parent));
}

/// <summary>
/// Adds a new symbol
/// </summary>
/// <param name="name">Name of the symbol.</param>
/// <param name="node">AST node in which the symbol is defined.</param>
void SymbolScope::add(const std::string& name, Ref<AstNode> node)
{
	assert(!name.empty());
	assert(m_symbols.count(name) == 0);

	m_symbols[name] = node;
}

/// <summary>
/// Checks if the name is already in scope. Checks parent scopes.
/// </summary>
/// <param name="name"></param>
/// <returns></returns>
bool SymbolScope::contains(const std::string& name)const
{
	if (m_symbols.count(name) > 0)
		return true;
	else if (m_parent.notNull())
		return m_parent->contains(name);
	else
		return false;
}

/// <summary>
/// Looks for a symbols.
/// </summary>
/// <param name="name"></param>
/// <param name="solveAlias">If true, alias nodes are not returned. Instead, 
/// the alias is solved and its destination node is returned.</param>
/// <returns></returns>
Ref<AstNode> SymbolScope::get(const std::string& name, bool solveAlias)const
{
	auto it = m_symbols.find(name);

	if (it == m_symbols.end())
	{
		if (m_parent.notNull())
			return m_parent->get(name, solveAlias);
		else
			return Ref<AstNode>();
	}
	else
	{
		auto node = it->second;

		if (solveAlias)
		{
			if (node->getType() == AST_TYPEDEF)
			{
				assert(node->childExists(0));
				node = node->children().front();

				if (node->getType() == AST_TYPE_NAME)
					node = this->get(node->getName(), true);
			}
		}

		return node;
	}
}



SymbolScope::SymbolScope(Ref<SymbolScope> parent) : m_parent(parent)
{
}


SymbolScope::~SymbolScope()
{
}

