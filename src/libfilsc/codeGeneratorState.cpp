#include "pch.h"
#include "codeGeneratorState.h"
#include "utils.h"

using namespace std;

/// <summary>
/// Gets the name in 'C' source for the given AST node.
/// </summary>
/// <param name="node"></param>
/// <returns></returns>
std::string CodeGeneratorState::cname(AstNode* node)
{
    if (node->hasFlag(ASTF_EXTERN_C))
        return node->getName();

    //This 'switch' handles special cases.
    switch (node->getType())
    {
    case AST_TYPEDEF:
        return cname(node->child(0).getPointer());

    case AST_DEFAULT_TYPE:
        if (node->getName() == "Cpointer")
            return "void *";
        else
            return node->getName();

    case AST_TYPE_NAME:
        return cname(node->getDataType());

    case AST_MESSAGE_TYPE:
        return "MessageSlot";

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

std::string CodeGeneratorState::cname(Ref<AstNode> node)
{
    return cname(node.getPointer());
}

/// <summary>
/// Checks if the type already has an assigned 'C' name. 
/// It does not try to create a name if it does not exist.
/// </summary>
/// <param name="type"></param>
/// <returns></returns>
bool CodeGeneratorState::hasName(AstNode* type)const
{
    if (type->getType() == AST_DEFAULT_TYPE)
        return true;
    else
        return m_objNames.count(type) > 0;
}


/// <summary>Forces to use a given name in the 'C' source for a given node.</summary>
/// <remarks>It is used to identify the entry point.</remarks>
/// <param name="node"></param>
/// <param name="name"></param>
void CodeGeneratorState::setCname(Ref<AstNode> node, const std::string& name)
{
    m_objNames[node] = name;

    switch (node->getType())
    {
    case AST_TUPLE_DEF:
    case AST_DEFAULT_TYPE:
    case AST_TYPE_NAME:
    case AST_ACTOR:
        //For this node types, also its data type gets the name.
        m_objNames[node->getDataType()] = name;
        break;
    default:
        break;
    }
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
bool CodeGeneratorState::allocTemp(const std::string& cTypeName, std::string& outputName, bool ref)
{
    auto tempVar = findTemporary([&cTypeName, ref](const TempVarInfo& varInfo) {
        return varInfo.cType == cTypeName
            && varInfo.ref == ref
            && varInfo.free == true;
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
        m_blockStack.back().tempVars.push_back(TempVarInfo{ cTypeName, outputName, ref });

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
            base = base.substr(0, 7) + "__" + base.substr(base.size() - 7, 7);

        replaceIn(base, '\'', "1");	// single quotes are illegal in 'C' names.
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
TempVariable::TempVariable(AstNode* type, CodeGeneratorState& state, bool ref)
    : IVariableInfo(ref), m_state(state), m_dataType(type)
{
    string	cTypeName = state.cname(type);

    if (state.allocTemp(cTypeName, m_cName, ref))
    {
        state.output() << cTypeName;
        if (ref)
            state.output() << "*";
        state.output() << "\t" << m_cName << ";\n";
    }

    //If 'allocTemp' returns false, it means that a temp value has been reused.
    //It shall not be declared again.
}

/// <summary>
/// Constructor of 'TempVariable'. Reserves a new temporary value.
/// </summary>
/// <param name="node"></param>
/// <param name="state"></param>
TempVariable::TempVariable(Ref<AstNode> node, CodeGeneratorState& state, bool ref)
    :TempVariable(node->getDataType(), state, ref)
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

AstNode* VoidVariable::dataType()const
{
    return astGetVoid();
}

NamedVariable::NamedVariable(Ref<AstNode> node, CodeGeneratorState& state)
    :IVariableInfo(false), m_node(node), m_cName(state.cname(node))
{
    if (m_node->hasFlag(ASTF_ACTOR_MEMBER))
        m_cName = "_gen_actor->" + m_cName;
}

const std::string& NamedVariable::cname()const
{
    return m_cName;
}
AstNode* NamedVariable::dataType()const
{
    return m_node->getDataType();
}

/// <summary>
/// Constructor for 'TupleField'. Resolves needed data on invocation.
/// </summary>
TupleField::TupleField(const IVariableInfo& tuple, int fieldIndex, CodeGeneratorState& state)
    :IVariableInfo(false)
{
    auto type = tuple.dataType();

    assert(astIsTupleType(type));

    assert(fieldIndex < (int)type->childCount());

    auto fieldNode = type->child(fieldIndex);
    m_type = fieldNode->getDataType();
    m_cName = tuple.cname() + "." + state.cname(fieldNode);
}

const std::string& TupleField::cname()const
{
    return m_cName;
}
AstNode* TupleField::dataType()const
{
    return m_type;
}
