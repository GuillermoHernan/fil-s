#pragma once

#include "ast.h"

/// <summary>Stores code generator state</summary>
/// <remarks>Most code generator state has to do with assigning names in 'C' source
/// to FILS variables, and managing local temporary variables.</remarks>
class CodeGeneratorState
{
public:
	CodeGeneratorState();
	~CodeGeneratorState();

	std::string cname(Ref<AstNode> node);

	void enterBlock();
	void exitBlock();

	bool allocTemp(const std::string& cTypeName, std::string& outputName);
	bool releaseTemp(const std::string& varName);

private:
	/// <summary>Info about a temporary variable.</summary>
	struct TempVarInfo
	{
		std::string	cType;
		std::string	cName;
		bool		free = false;
	};

	/// <summary> Keeps track of the temporaries of a block</summary>
	struct BlockInfo
	{
		std::vector<TempVarInfo>	tempVars;
	};

	std::vector<BlockInfo>					m_blockStack;
	std::map< Ref<AstNode>, std::string>	m_nodeCNames;
	int										m_nextSymbolId = 0;

	std::string		allocCName(const std::string& base);
	TempVarInfo*	findTemporary(std::function<bool(const TempVarInfo&)> predicate);
};


/// <summary>
/// Manages the life time of a temporary value
/// </summary>
class TempVariable
{
public:
	TempVariable(Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state);
	~TempVariable();

	const std::string& cname()const
	{
		return m_cName;
	}

private:
	std::string			m_cName;
	CodeGeneratorState& m_state;
};