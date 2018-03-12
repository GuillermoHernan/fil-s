/// <summary>
/// Abstract Syntax Tree classes / functions
/// </summary>

#include "pch.h"
#include "ast.h"
#include "SymbolScope.h"
#include "dataTypes.h"

using namespace std;

//Global AST node count.
int AstNode::ms_nodeCount = 0;

/// <summary>
/// Creates an AST node.
/// </summary>
/// <param name="type">Node type.</param>
/// <param name="pos">Position of source code which originated the node.</param>
/// <param name="name">Node name (can be empty)</param>
/// <param name="value">Node value (can be empty)</param>
/// <param name="flags">Node flags.</param>
/// <returns></returns>
Ref<AstNode> AstNode::create(
	AstNodeTypes type,
	ScriptPosition pos,
	const std::string& name,
	const std::string& value,
	int flags
)
{
	return refFromNew(new AstNode(type, pos, name, value, flags));
}


AstNode::AstNode(
	AstNodeTypes type,
	const ScriptPosition& pos,
	const std::string& name,
	const std::string& value,
	int flags)
	:m_position(pos), m_type(type), m_name(name), m_value(value), m_flags(flags)
{
	m_dataType = DefaultType::createVoid();
	++ms_nodeCount;
}

/// <summary>
/// Removes the references to the objects which may create circular refrences to the AST 
/// tree. Basically, the scope and the DataType.
/// </summary>
void AstNode::destroy()
{
	m_dataType.reset();
	m_scope.reset();

	for (auto child : children())
	{
		if (child.notNull())
			child->destroy();
	}
}

Ref<SymbolScope> AstNode::getScope()const
{
	return m_scope.staticCast<SymbolScope>();
}

void AstNode::setScope(Ref<SymbolScope> scope)
{
	m_scope = scope.staticCast<SymbolScope>();
}


Ref<BaseType> AstNode::getDataType()const
{
	return m_dataType.staticCast<BaseType>();
}

void AstNode::setDataType(Ref<BaseType> dataType)
{
	m_dataType = dataType;
}


//  Functions to create specific AST node types.
//
////////////////////////////////


Ref<AstNode> astCreateModule(const std::string& name)
{
	return AstNode::create (AST_MODULE, ScriptPosition(), name, "");
}

Ref<AstNode> astCreateScript(ScriptPosition pos, const std::string& name)
{
	return AstNode::create(AST_SCRIPT, pos, name, "");
}

/// <summary>
/// Creates a generic variable declaration node.
/// Access type (var, const) is not specified.
/// </summary>
/// <param name="token"></param>
/// <param name="typeDesc"></param>
/// <param name="initExpr"></param>
/// <returns></returns>
Ref<AstNode> astCreateDeclaration(LexToken token,
	Ref<AstNode> typeDesc,
	Ref<AstNode> initExpr)
{
	return astCreateDeclaration(token.getPosition(), token.text(), typeDesc, initExpr);
}

/// <summary>
/// Creates a generic variable declaration node.
/// Access type (var, const) is not specified.
/// </summary>
/// <param name="token"></param>
/// <param name="typeDesc"></param>
/// <param name="initExpr"></param>
/// <returns></returns>
Ref<AstNode> astCreateDeclaration(ScriptPosition pos,
	const std::string& name,
	Ref<AstNode> typeDesc,
	Ref<AstNode> initExpr)
{
	auto result = AstNode::create(AST_DECLARATION, pos, name, "");

	result->addChild(typeDesc);
	result->addChild(initExpr);
	return result;
}


/// <summary>
/// Creates a type definition node.
/// </summary>
/// <param name="pos"></param>
/// <param name="name"></param>
/// <param name="typeDesc"></param>
/// <returns></returns>
Ref<AstNode> astCreateTypedef(ScriptPosition pos, const std::string& name, Ref<AstNode> typeDesc)
{
	auto result = AstNode::create(AST_TYPEDEF, pos, name, "");

	result->addChild(typeDesc);
	return result;
}


/// <summary>
/// Creates a function definition AST node.
/// </summary>
/// <param name="pos"></param>
/// <param name="name"></param>
/// <param name="params"></param>
/// <param name="returnType"></param>
/// <param name="bodyExpr"></param>
/// <returns></returns>
Ref<AstNode> astCreateFunction(ScriptPosition pos,
	const std::string& name,
	Ref<AstNode> params,
	Ref<AstNode> returnType,
	Ref<AstNode> bodyExpr)
{
	auto result = AstNode::create(AST_FUNCTION, pos, name, "");

	result->addChild(params);
	result->addChild(returnType);
	result->addChild(bodyExpr);

	return result;
}


Ref<AstNode> astCreateBlock(LexToken token)
{
	return AstNode::create(AST_BLOCK, token.getPosition(), "", "");
}

Ref<AstNode> astCreateTuple(LexToken token)
{
	return AstNode::create(AST_TUPLE, token.getPosition(), "", "");
}

Ref<AstNode> astCreateTupleDef(ScriptPosition pos, const std::string& name)
{
	return AstNode::create(AST_TUPLE, pos, name, "");
}

Ref<AstNode> astCreateTupleAdapter(Ref<AstNode> tupleNode)
{
	auto result = AstNode::create(AST_TUPLE_ADAPTER, tupleNode->position(), "", "");

	result->addChild(tupleNode);
	return result;
}


Ref<AstNode> astCreateIf (ScriptPosition pos, 
                          Ref<AstNode> condition,
                          Ref<AstNode> thenSt,
                          Ref<AstNode> elseSt)
{
	auto result = AstNode::create(AST_IF, pos, "", "");

    result->addChild(condition);
    result->addChild(thenSt);
    result->addChild(elseSt);
    
    return result;
}

Ref<AstNode> astCreateFor (ScriptPosition pos, 
                          Ref<AstNode> initSt,
                          Ref<AstNode> condition,
                          Ref<AstNode> incrementSt,
                          Ref<AstNode> body)
{    
	auto result = AstNode::create(AST_FOR, pos, "", "");
    
    result->addChild(initSt);
    result->addChild(condition);
    result->addChild(incrementSt);
    result->addChild(body);

    return result;
}

Ref<AstNode> astCreateForEach (ScriptPosition pos, 
                          Ref<AstNode> itemDeclaration,
                          Ref<AstNode> sequenceExpr,
                          Ref<AstNode> body)
{
	auto result = AstNode::create(AST_FOR_EACH, pos, "", "");

    result->addChild(itemDeclaration);
    result->addChild(sequenceExpr);
    result->addChild(body);

    return result;
}


Ref<AstNode> astCreateReturn (ScriptPosition pos, Ref<AstNode> expr)
{
	auto result = AstNode::create(AST_RETURN, pos, "", "");
    
    result->addChild(expr);
    return result;
}

/// <summary>
/// Creates and assignment node.
/// </summary>
/// <param name="opToken"></param>
/// <param name="lexpr"></param>
/// <param name="rexpr"></param>
/// <returns></returns>
Ref<AstNode> astCreateAssignment(LexToken opToken,
                                 Ref<AstNode> lexpr, 
                                 Ref<AstNode> rexpr)
{
	auto result = AstNode::create(AST_ASSIGNMENT, opToken.getPosition(), "", opToken.text());
    
    result->addChild(lexpr);
    result->addChild(rexpr);
    return result;
}

Ref<AstNode> astCreatePrefixOp(LexToken token, Ref<AstNode> rexpr)
{
	auto result = AstNode::create(AST_PREFIXOP, token.getPosition(), "", token.text());
    
    result->addChild(rexpr);
    return result;
}

Ref<AstNode> astCreatePostfixOp(LexToken token, Ref<AstNode> lexpr)
{
	auto result = AstNode::create(AST_POSTFIXOP, token.getPosition(), "", token.text());
    
    result->addChild(lexpr);
    return result;
}

Ref<AstNode> astCreateBinaryOp(LexToken token, 
                                 Ref<AstNode> lexpr, 
                                 Ref<AstNode> rexpr)
{
	auto result = AstNode::create(AST_BINARYOP, token.getPosition(), "", token.text());
    
    result->addChild(lexpr);
    result->addChild(rexpr);
    return result;
}

Ref<AstNode> astCreateFnCall(ScriptPosition pos, Ref<AstNode> fnExpr, Ref<AstNode> params)
{
	auto result = AstNode::create(AST_FNCALL, pos);
    
	result->addChild(fnExpr);
	result->addChild(params);
	return result;
}

/**
 * Creates an array literal AST node.
 * @param pos
 * @return 
 */
Ref<AstNode> astCreateArray(ScriptPosition pos)
{
	return AstNode::create(AST_ARRAY, pos);
}

Ref<AstNode> astCreateArrayAccess(ScriptPosition pos,
                                  Ref<AstNode> arrayExpr, 
                                  Ref<AstNode> indexExpr)
{    
	auto result = AstNode::create(AST_ARRAY_ACCESS, pos);

    result->addChild(arrayExpr);
    result->addChild(indexExpr);

    return result;
}

Ref<AstNode> astCreateMemberAccess(ScriptPosition pos,
                                  Ref<AstNode> objExpr, 
                                  Ref<AstNode> identifier)
{    
	auto result = AstNode::create(AST_MEMBER_ACCESS, pos);
    
    result->addChild(objExpr);
    result->addChild(identifier);

    return result;
}

Ref<AstNode> astCreateActor(ScriptPosition pos, const std::string& name)
{
	return AstNode::create(AST_ACTOR, pos, name);
}

Ref<AstNode> astCreateInputMsg(ScriptPosition pos, const std::string& name)
{
	return AstNode::create(AST_INPUT, pos, name);
}

Ref<AstNode> astCreateOutputMsg(ScriptPosition pos, const std::string& name)
{
	return AstNode::create(AST_OUTPUT, pos, name);
}

/**
 * Creates an 'AstLiteral' object from a source token.
 * @param token
 * @return 
 */
Ref<AstNode> astCreateLiteral(LexToken token)
{
	string	value;
	auto	pos = token.getPosition();

	if (token.type() == LEX_STR)
		value = token.strValue();
	else
		value = token.text();

    switch (token.type())
    {
	case LEX_STR:	return AstNode::create(AST_STRING, pos, "", value);
    case LEX_INT:	return AstNode::create(AST_INTEGER, pos, "", value);
	case LEX_FLOAT:	return AstNode::create(AST_FLOAT, pos, "", value);
        
    default:
        assert(!"Invalid token for a literal");
		return Ref<AstNode>();
    }
}

/// <summary>
/// Creates an 'AStLiteral' from a boolean value.
/// </summary>
/// <param name="pos"></param>
/// <param name="value"></param>
/// <returns></returns>
Ref<AstNode> astCreateBool(ScriptPosition pos, bool value)
{
	const char* strValue = value ? "1" : "0";
	return AstNode::create ( AST_BOOL, pos, "", strValue);
}

/// <summary>
/// Creates an AST node for a default (predefined) data type.
/// </summary>
/// <returns></returns>
Ref<AstNode> astCreateDefaultType(Ref<DefaultType> type)
{
	auto node = AstNode::create(AST_DEFAULT_TYPE, ScriptPosition(), type->getName());

	node->setDataType(type);
	return node;
}

/// <summary>
/// Creates an unnamed input AST node.
/// </summary>
/// <param name="pos"></param>
/// <param name="outputPath"></param>
/// <param name="params"></param>
/// <param name="code"></param>
/// <returns></returns>
Ref<AstNode> astCreateUnnamedInput(ScriptPosition pos,
	Ref<AstNode> outputPath,
	Ref<AstNode> params,
	Ref<AstNode> code)
{
	auto result = AstNode::create(AST_UNNAMED_INPUT, pos);

	result->addChild(outputPath);
	result->addChild(params);
	result->addChild(code);

	return result;
}

/// <summary>
/// Gets the string representation of an AST type
/// </summary>
/// <param name="type"></param>
/// <returns></returns>
std::string astTypeToString(AstNodeTypes type)
{
    typedef map<AstNodeTypes, string>   TypesMap;
    static TypesMap types;
    
    if (types.empty())
    {
		types[AST_MODULE] = "AST_MODULE";
		types[AST_SCRIPT] = "AST_SCRIPT";
		types[AST_TYPEDEF] = "AST_TYPEDEF";
		types[AST_LIST] = "AST_LIST";
		types[AST_BLOCK] = "AST_BLOCK";
		types[AST_TUPLE] = "AST_TUPLE";
		types[AST_DECLARATION] = "AST_DECLARATION";
		types[AST_TUPLE_DEF] = "AST_TUPLE_DEF";
		types[AST_TUPLE_ADAPTER] = "AST_TUPLE_ADAPTER";
		types[AST_IF] = "AST_IF";
		types[AST_FOR] = "AST_FOR";
		types[AST_FOR_EACH] = "AST_FOR_EACH";
		types[AST_RETURN] = "AST_RETURN";
		types[AST_FUNCTION] = "AST_FUNCTION";
		types[AST_ASSIGNMENT] = "AST_ASSIGNMENT";
		types[AST_FNCALL] = "AST_FNCALL";
		types[AST_INTEGER] = "AST_INTEGER";
		types[AST_FLOAT] = "AST_FLOAT";
		types[AST_STRING] = "AST_STRING";
		types[AST_BOOL] = "AST_BOOL";
		types[AST_IDENTIFIER] = "AST_IDENTIFIER";
		types[AST_ARRAY] = "AST_ARRAY";
		types[AST_ARRAY_ACCESS] = "AST_ARRAY_ACCESS";
		types[AST_MEMBER_ACCESS] = "AST_MEMBER_ACCESS";
		types[AST_MEMBER_NAME] = "AST_MEMBER_NAME";
		types[AST_BINARYOP] = "AST_BINARYOP";
		types[AST_PREFIXOP] = "AST_PREFIXOP";
		types[AST_POSTFIXOP] = "AST_POSTFIXOP";
		types[AST_ACTOR] = "AST_ACTOR";
		types[AST_DEFAULT_TYPE] = "AST_DEFAULT_TYPE";
		types[AST_TYPE_NAME] = "AST_TYPE_NAME";
		types[AST_INPUT] = "AST_INPUT";
		types[AST_OUTPUT] = "AST_OUTPUT";
		types[AST_UNNAMED_INPUT] = "AST_UNNAMED_INPUT";
		types[AST_IMPORT] = "AST_IMPORT";
		//types[AST_TYPES_COUNT] = "AST_TYPES_COUNT";

		assert(types.size() == AST_TYPES_COUNT);
    }
    
    TypesMap::const_iterator it = types.find(type);
    
	if (it != types.end())
		return it->second;
	else
		return "BAD_AST_TYPE";
}

/// <summary>
/// Gets an AST node type from its string representation
/// </summary>
/// <param name="str"></param>
/// <returns></returns>
AstNodeTypes astTypeFromString(const std::string& str)
{
	typedef map<string, AstNodeTypes>   TypesMap;
	static TypesMap types;

	if (types.empty())
	{
		types["AST_MODULE"] = AST_MODULE;
		types["AST_SCRIPT"] = AST_SCRIPT;
		types["AST_TYPEDEF"] = AST_TYPEDEF;
		types["AST_LIST"] = AST_LIST;
		types["AST_BLOCK"] = AST_BLOCK;
		types["AST_TUPLE"] = AST_TUPLE;
		types["AST_DECLARATION"] = AST_DECLARATION;
		types["AST_TUPLE_DEF"] = AST_TUPLE_DEF;
		types["AST_TUPLE_ADAPTER"] = AST_TUPLE_ADAPTER;
		types["AST_IF"] = AST_IF;
		types["AST_FOR"] = AST_FOR;
		types["AST_FOR_EACH"] = AST_FOR_EACH;
		types["AST_RETURN"] = AST_RETURN;
		types["AST_FUNCTION"] = AST_FUNCTION;
		types["AST_ASSIGNMENT"] = AST_ASSIGNMENT;
		types["AST_FNCALL"] = AST_FNCALL;
		types["AST_INTEGER"] = AST_INTEGER;
		types["AST_FLOAT"] = AST_FLOAT;
		types["AST_STRING"] = AST_STRING;
		types["AST_BOOL"] = AST_BOOL;
		types["AST_IDENTIFIER"] = AST_IDENTIFIER;
		types["AST_ARRAY"] = AST_ARRAY;
		types["AST_ARRAY_ACCESS"] = AST_ARRAY_ACCESS;
		types["AST_MEMBER_ACCESS"] = AST_MEMBER_ACCESS;
		types["AST_MEMBER_NAME"] = AST_MEMBER_NAME;
		types["AST_BINARYOP"] = AST_BINARYOP;
		types["AST_PREFIXOP"] = AST_PREFIXOP;
		types["AST_POSTFIXOP"] = AST_POSTFIXOP;
		types["AST_ACTOR"] = AST_ACTOR;
		types["AST_DEFAULT_TYPE"] = AST_DEFAULT_TYPE;
		types["AST_TYPE_NAME"] = AST_TYPE_NAME;
		types["AST_INPUT"] = AST_INPUT;
		types["AST_OUTPUT"] = AST_OUTPUT;
		types["AST_UNNAMED_INPUT"] = AST_UNNAMED_INPUT;
		types["AST_IMPORT"] = AST_IMPORT;
		//types[AST_TYPES_COUNT"] = AST_TYPES_COUNT";

		assert(types.size() == AST_TYPES_COUNT);
	}

	auto it = types.find(str);

	if (it != types.end())
		return it->second;
	else
	{
		string message = "Unknown AST type string: " + str;
		throw exception(message.c_str());
	}
}
