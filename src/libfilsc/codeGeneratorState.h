#pragma once

#include "ast.h"
#include "dataTypes.h"

#ifndef FRIEND_TEST
#define FRIEND_TEST(x,y)
#endif

/// <summary>Stores code generator state</summary>
/// <remarks>Most code generator state has to do with assigning names in 'C' source
/// to FILS variables, and managing local temporary variables.</remarks>
class CodeGeneratorState
{
public:
	//typedef void(*TypeCodegenFN)(Ref<BaseType> type, CodeGeneratorState& state);
	typedef std::function<void(Ref<BaseType>, CodeGeneratorState&)> TypeCodegenFN;

	CodeGeneratorState(std::ostream* pOutput, TypeCodegenFN typeGenFN);
	~CodeGeneratorState();

	CodeGeneratorState(const CodeGeneratorState&) = delete;
	CodeGeneratorState& operator=(const CodeGeneratorState&) = delete;

	std::string cname(Ref<AstNode> node);
	std::string cname(Ref<BaseType> type);
	bool hasName(Ref<BaseType> type)const;

	void setCname(Ref<AstNode> node, const std::string& name);

	std::ostream& output()
	{
		return *m_output;
	}

	void typeCodegen(Ref<BaseType> type, CodeGeneratorState& state);

protected:
	void enterBlock();
	void exitBlock();

	bool allocTemp(const std::string& cTypeName, std::string& outputName, bool ref);
	bool releaseTemp(const std::string& varName);

	friend class TempVariable;
	friend class CodegenBlock;
	FRIEND_TEST(CodeGeneratorState, temporaries);

private:
	/// <summary>Info about a temporary variable.</summary>
	struct TempVarInfo
	{
		const std::string	cType;
		const std::string	cName;
		const bool			ref;
		bool				free = false;
	};

	/// <summary> Keeps track of the temporaries of a block</summary>
	struct BlockInfo
	{
		std::vector<TempVarInfo>	tempVars;
	};

	std::ostream*								m_output;
	TypeCodegenFN								m_typeGenFN;
	std::vector<BlockInfo>						m_blockStack;
	std::map< Ref<RefCountObj>, std::string>	m_objNames;
	int											m_nextSymbolId = 0;

	std::string		allocCName(std::string base);
	TempVarInfo*	findTemporary(std::function<bool(const TempVarInfo&)> predicate);
};

/// <summary>
/// Manages the lifetime of a block in code generation.
/// </summary>
class CodegenBlock
{
public:
	CodegenBlock(CodeGeneratorState& state) : m_state(state)
	{
		state.enterBlock();
	}

	~CodegenBlock()
	{
		m_state.exitBlock();
	}

private:
	CodeGeneratorState& m_state;
};


/// <summary>
/// Interface to get informaiton about variables in code generation.
/// </summary>
struct IVariableInfo
{
	virtual const std::string&	cname()const=0;
	virtual Ref<BaseType>		dataType()const=0;

	bool isVoid()const
	{
		return dataType()->type() == DT_VOID;
	}

	const bool isReference;

protected:
	IVariableInfo(bool ref) : isReference(ref) {}
};

std::ostream& operator << (std::ostream& output, const IVariableInfo& var);


/// <summary>
/// Manages the life time of a temporary value
/// </summary>
class TempVariable : public IVariableInfo
{
public:
	TempVariable(Ref<BaseType> type, CodeGeneratorState& state, bool ref);
	TempVariable(Ref<AstNode> node, CodeGeneratorState& state, bool ref);
	~TempVariable();

	const std::string& cname()const override
	{
		return m_cName;
	}

	virtual Ref<BaseType> dataType()const override
	{
		return m_dataType;
	}

private:
	std::string			m_cName;
	Ref<BaseType>		m_dataType;
	CodeGeneratorState& m_state;
};

/// <summary>
/// Utility class to create void variables in code generation.
/// </summary>
struct VoidVariable : public IVariableInfo
{
	virtual const std::string&	cname()const override;
	virtual Ref<BaseType>		dataType()const  override;

	VoidVariable() : IVariableInfo(false) {}
};

/// <summary>
/// Class to identify named variables in code generation.
/// </summary>
class NamedVariable : public IVariableInfo
{
public:
	NamedVariable(Ref<AstNode> node, CodeGeneratorState& state);

	virtual const std::string&	cname()const override;
	virtual Ref<BaseType>		dataType()const  override;

private:
	Ref<AstNode>		m_node;
	std::string			m_cName;
};

/// <summary>
/// Class to identify a tuple field.
/// </summary>
class TupleField : public IVariableInfo
{
public:
	TupleField(const IVariableInfo& tuple, int fieldIndex, CodeGeneratorState& state);

	virtual const std::string&	cname()const override;
	virtual Ref<BaseType>		dataType()const  override;

private:
	Ref<BaseType>		m_type;
	std::string			m_cName;
};