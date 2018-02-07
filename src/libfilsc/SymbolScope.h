/// <summary>
/// Implementation of symbol scope: stores the symbols used in the compiled program
/// in a hierarchical way.
/// </summary>
#pragma once

#include "ast.h"

/// /// <summary>
/// Stores program symbols in a hierarchical way
/// </summary>
class SymbolScope : public RefCountObj
{
public:
	static Ref<SymbolScope> create(Ref<SymbolScope> parent);

	void add(const std::string& name, Ref<AstNode> node);

	bool			contains(const std::string& name, bool checkParents=true)const;
	Ref<AstNode>	get(const std::string& name, bool solveAlias = false)const;

protected:
	SymbolScope(Ref<SymbolScope> parent);
	~SymbolScope();

private:
	std::map<std::string, Ref<AstNode>>		m_symbols;
	Ref<SymbolScope>						m_parent;
};

