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
bool SymbolScope::contains(const std::string& name)
{
	if (m_symbols.count(name) > 0)
		return true;
	else if (m_parent.notNull())
		return m_parent->contains(name);
	else
		return false;
}


SymbolScope::SymbolScope(Ref<SymbolScope> parent) : m_parent(parent)
{
}


SymbolScope::~SymbolScope()
{
}

