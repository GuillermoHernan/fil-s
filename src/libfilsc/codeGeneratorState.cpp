#include "pch.h"
#include "codeGeneratorState.h"

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
	case AST_FUNCTION:
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



/// <summary>
/// Constructor of CodeGeneratorState
/// </summary>
CodeGeneratorState::CodeGeneratorState()
{
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
		outputName = allocCName("temp_");
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
std::string	CodeGeneratorState::allocCName(const std::string& base)
{
	const size_t	bufSize = base.size() + 32;
	char *			buffer = new char[bufSize];
	const char*		base2 = base.c_str();

	if (base != "")
		base2 = "_unnamed";

	sprintf_s(buffer, bufSize, "%s_%04X", base2, m_nextSymbolId++);

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
/// Constructor of 'TempVariable'. Reserves a new temporary value.
/// </summary>
/// <param name="node"></param>
/// <param name="output"></param>
/// <param name="state"></param>
TempVariable::TempVariable(Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state)
	:m_state(state)
{
	auto	typeNode = node->getDataType();
	string	cTypeName = state.cname(typeNode);

	if (state.allocTemp(cTypeName, m_cName))
		output << cTypeName << "\t" << m_cName << ";\n";

	//If 'allocTemp' returns false, it means that a temp value has been reused.
	//It shall not be declared again.
}

/// <summary>
/// Destructor. Releases the temporary variable.
/// </summary>
TempVariable::~TempVariable()
{
	m_state.releaseTemp(m_cName);
}
