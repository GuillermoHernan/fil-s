#include "pch.h"
#include "codeGeneratorState.h"
#include "utils.h"

using namespace std;

/// <summary>
/// Gets the name in 'C' source for the given AST node.
/// </summary>
/// <param name="node"></param>
/// <returns></returns>
std::string CodeGeneratorState::cname(Ref<AstNode> node)
{
	//This 'switch' handles special cases.
	switch (node->getType())
	{
	case AST_TYPEDEF:
		return cname(node->child(0));
	
	case AST_TUPLE_DEF:
	case AST_DEFAULT_TYPE:
	case AST_TYPE_NAME:
	case AST_ACTOR:
		return cname(node->getDataType());

	default:
		break;
	}

	auto it = m_objNames.find(node);

	if (it != m_objNames.end())
		return it->second;
	else
	{
		string name = allocCName(node->getName());

		m_objNames[node] = name;
		return name;
	}
}

/// <summary>
/// Gets the name in 'C' source for the given data type.
/// </summary>
/// <param name="type"></param>
/// <returns></returns>
std::string CodeGeneratorState::cname(Ref<BaseType> type)
{
	auto it = m_objNames.find(type);

	if (it != m_objNames.end())
		return it->second;
	else
	{
		string name;

		if (type->type() == DT_BOOL || type->type() == DT_INT)
			name = type->getName();
		else
			name = allocCName(type->getName());

		m_objNames[type] = name;
		return name;
	}
}

/// <summary>Forces to use a given name in the 'C' source for a given node.</summary>
/// <remarks>It is used to identify the entry point.</remarks>
/// <param name="node"></param>
/// <param name="name"></param>
void CodeGeneratorState::setCname(Ref<AstNode> node, const std::string& name)
{
	m_objNames[node] = name;
}

/// <summary>
/// Constructor of CodeGeneratorState
/// </summary>
CodeGeneratorState::CodeGeneratorState(std::ostream* pOutput)
	: m_output(pOutput)
{
	assert(pOutput != NULL);
	//Create root block
	enterBlock();
}

CodeGeneratorState::~CodeGeneratorState()
{
	//Check that only the root block remains in the stack
	assert(m_blockStack.size() == 1);
}

/// <summary>Enters in a new block.</summary>
/// <remarks>Each block stores its own temporaries</remarks>
void CodeGeneratorState::enterBlock()
{
	m_blockStack.push_back(BlockInfo());
}

/// <summary>
/// Exit of a block. All temporaries are released.
/// </summary>
void CodeGeneratorState::exitBlock()
{
	m_blockStack.pop_back();

	//Root block shall not be destroyed.
	assert(m_blockStack.size() > 0);
}

/// <summary>
/// Allocates a new temporary variable.
/// </summary>
/// <param name="cTypeName">Type name (in 'C' code)</param>
/// <param name="outputName">In this variable the generated
/// name is copied (output parameter)</param>
/// <returns>true if a new temporary has been allocated, or false if a previous
/// temporary has been reused</returns>
bool CodeGeneratorState::allocTemp(const std::string& cTypeName, std::string& outputName)
{
	auto tempVar = findTemporary([&cTypeName](const TempVarInfo& varInfo) {
		return varInfo.cType == cTypeName && varInfo.free == true;
	});

	//Try to reuse.
	if (tempVar != NULL)
	{
		tempVar->free = false;
		outputName = tempVar->cName;

		return false;
	}
	else
	{
		outputName = allocCName("temp");
		m_blockStack.back().tempVars.push_back(TempVarInfo{ cTypeName, outputName });

		return true;
	}
}

/// <summary>
/// Indicates that a temporary variable is no longer used.
/// </summary>
/// <param name="varName"></param>
/// <returns>true if the variable exists and was used. 'false' in other case.</returns>
bool CodeGeneratorState::releaseTemp(const std::string& varName)
{
	auto tempVar = findTemporary([&varName](const TempVarInfo& varInfo) {
		return varInfo.cName == varName;
	});

	if (tempVar != NULL)
	{
		bool result = !tempVar->free;
		tempVar->free = true;
		return result;
	}
	else
		return false;
}

/// <summary>
/// Creates a new, unique name for generated 'C' source.
/// </summary>
/// <param name="base">The name will start by this root</param>
/// <returns></returns>
std::string	CodeGeneratorState::allocCName(std::string base)
{
	const size_t	bufSize = base.size() + 32;
	char *			buffer = new char[bufSize];

	if (base == "")
		base = "_unnamed";
	else
	{
		//Limit name length
		if (base.size() > 16)
			base = base.substr(0, 7) + "__" + base.substr(base.size()-7, 7);

		replace(base, '\'', "1");	// single quotes are illegal in 'C' names.
	}

	sprintf_s(buffer, bufSize, "%s_%04X", base.c_str(), m_nextSymbolId++);

	string result = buffer;
	delete[]buffer;

	return result;
}

/// <summary>
/// Looks for the first temporary which fulfills the predicate in the current block.
/// </summary>
/// <param name="predicate"></param>
/// <returns></returns>
CodeGeneratorState::TempVarInfo* CodeGeneratorState::findTemporary(
	std::function<bool(const TempVarInfo&)> predicate)
{
	for (TempVarInfo& varInfo : m_blockStack.back().tempVars)
	{
		if (predicate(varInfo))
			return &varInfo;
	}

	return NULL;
}

/// <summary>
/// Utility operator to write the 'C' name of a variable on the output stream.
/// </summary>
std::ostream& operator << (std::ostream& output, const IVariableInfo& var)
{
	output << var.cname();
	return output;
}

/// <summary>
/// Constructor of 'TempVariable'. Reserves a new temporary value.
/// </summary>
/// <param name="type"></param>
/// <param name="state"></param>
TempVariable::TempVariable(Ref<BaseType> type, CodeGeneratorState& state)
	:m_state(state), m_dataType(type)
{
	string	cTypeName = state.cname(type);

	if (state.allocTemp(cTypeName, m_cName))
		state.output() << cTypeName << "\t" << m_cName << ";\n";

	//If 'allocTemp' returns false, it means that a temp value has been reused.
	//It shall not be declared again.
}

/// <summary>
/// Constructor of 'TempVariable'. Reserves a new temporary value.
/// </summary>
/// <param name="node"></param>
/// <param name="state"></param>
TempVariable::TempVariable(Ref<AstNode> node, CodeGeneratorState& state)
	:TempVariable(node->getDataType(), state)
{
}


/// <summary>
/// Destructor. Releases the temporary variable.
/// </summary>
TempVariable::~TempVariable()
{
	m_state.releaseTemp(m_cName);
}


const std::string& VoidVariable::cname()const
{
	static string empty;
	return empty;
}

Ref<BaseType> VoidVariable::dataType()const
{
	return DefaultType::createVoid();
}

NamedVariable::NamedVariable(Ref<AstNode> node, CodeGeneratorState& state)
	:m_node(node), m_cName(state.cname(node))
{
}

const std::string& NamedVariable::cname()const
{
	return m_cName;
}
Ref<BaseType> NamedVariable::dataType()const
{
	return m_node->getDataType();
}

/// <summary>
/// Constructor for 'TupleField'. Resolves needed data on invocation.
/// </summary>
TupleField::TupleField(const IVariableInfo& tuple, int fieldIndex, CodeGeneratorState& state)
{
	auto type = tuple.dataType();

	assert(type->type() == DT_TUPLE);
	auto	tupleType = type.staticCast<TupleType>();

	assert(fieldIndex < tupleType->memberCount());

	m_type = tupleType->getMemberType(fieldIndex);
	m_cName = tuple.cname() + "." + state.cname(tupleType->getMemberNode(fieldIndex));
}

const std::string& TupleField::cname()const
{
	return m_cName;
}
Ref<BaseType> TupleField::dataType()const
{
	return m_type;
}
