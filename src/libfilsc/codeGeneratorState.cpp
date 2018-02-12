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
	auto it = m_nodeCNames.find(node);

	if (it != m_nodeCNames.end())
		return it->second;
	else
	{
		string name;

		if (node->getType() == AST_DEFAULT_TYPE)
		{
			//TODO: This is just valid for the current, limited datatypes.
			name = node->getName();
		}
		else
		{
			string base = node->getName();

			if (base == "")
				base = "_unnamed";

			name = allocCName(name);
		}

		m_nodeCNames[node] = name;
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

	sprintf_s(buffer, bufSize, "%s_%04X",base.c_str(), m_nextSymbolId++);

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
