/// <summary>
/// Abstract Syntax Tree classes / functions
/// This file contains mostly constructor functions for AST nodes, and some 
/// utility functions to query the AST.
/// </summary>

#include "pch.h"
#include "ast.h"
#include "dependencySolver.h"

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

/// <summary>
/// Ast node constructor.
/// </summary>
/// <param name="type">Type of AstNode</param>
/// <param name="pos">Position in the source code.</param>
/// <param name="name">Node name (can be empty)</param>
/// <param name="value">Node value (can be empty)</param>
/// <param name="flags">Flags. See 'AstFlags' enum</param>
AstNode::AstNode(
    AstNodeTypes type,
    const ScriptPosition& pos,
    const std::string& name,
    const std::string& value,
    int flags)
    :m_position(pos), m_type(type), m_name(name), m_value(value), m_flags(flags)
{
    //TODO: This has been done to prevent an infinite loop. Find another solution...
    //Also to fix the data type of default types...
    if (type == AST_TUPLE_DEF || type == AST_DEFAULT_TYPE)
        m_reference = this;
    else
        m_reference = astGetVoid();

    ++ms_nodeCount;
}

/// <summary>
/// Gets the node assigned data type.
/// </summary>
/// <returns></returns>
AstNode* AstNode::getDataType()const
{
    if (getType() == AST_IDENTIFIER)
        return m_reference->getDataType();
    else
        return m_reference;
}

/// <summary>
/// Changes the data type assigned to the node.
/// </summary>
/// <param name="dataType"></param>
void AstNode::setDataType(AstNode* dataType)
{
    assert(getType() != AST_IDENTIFIER);
    m_reference = dataType;
}

/// <summary>
/// Sets the referenced node. Only for identifiers
/// </summary>
/// <param name="node"></param>
void AstNode::setReference(AstNode* node)
{
    assert(getType() == AST_IDENTIFIER || getType() == AST_IMPORT);
    m_reference = node;
}

//  Functions to create specific AST node types.
//
////////////////////////////////


Ref<AstNode> astCreateModule(const std::string& name)
{
    return AstNode::create(AST_MODULE, ScriptPosition(), name, "");
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
/// Creates an array declaration node.
/// </summary>
/// <param name="pos"></param>
/// <param name="typeSpec"></param>
/// <param name="sizeExpr"></param>
/// <returns></returns>
Ref<AstNode> astCreateArrayDecl(ScriptPosition pos,
    Ref<AstNode> typeSpec,
    Ref<AstNode> sizeExpr)
{
    auto result = AstNode::create(AST_ARRAY_DECL, pos);

    result->addChild(typeSpec);
    result->addChild(sizeExpr);
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

/// <summary>
/// Creates a function type AST node.
/// </summary>
/// <returns></returns>
Ref<AstNode> astCreateFunctionType(ScriptPosition pos,
    Ref<AstNode> params,
    Ref<AstNode> returnType)
{
    auto result = AstNode::create(AST_FUNCTION_TYPE, pos);

    result->addChild(params);
    result->addChild(returnType);

    return result;
}


Ref<AstNode> astCreateBlock(LexToken token)
{
    return AstNode::create(AST_BLOCK, token.getPosition(), "", "");
}

Ref<AstNode> astCreateTuple(ScriptPosition pos)
{
    return AstNode::create(AST_TUPLE, pos, "", "");
}

Ref<AstNode> astCreateTupleDef(ScriptPosition pos, const std::string& name)
{
    return AstNode::create(AST_TUPLE_DEF, pos, name, "");
}

Ref<AstNode> astCreateTupleAdapter(Ref<AstNode> tupleNode)
{
    auto result = AstNode::create(AST_TUPLE_ADAPTER, tupleNode->position(), "", "");

    result->addChild(tupleNode);
    return result;
}


Ref<AstNode> astCreateIf(ScriptPosition pos,
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

Ref<AstNode> astCreateFor(ScriptPosition pos,
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

Ref<AstNode> astCreateForEach(ScriptPosition pos,
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


Ref<AstNode> astCreateReturn(ScriptPosition pos, Ref<AstNode> expr)
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


/// <summary>
/// Creates a message type AST node.
/// </summary>
/// <returns></returns>
Ref<AstNode> astCreateMessageType(ScriptPosition pos, Ref<AstNode> params)
{
    auto result = AstNode::create(AST_MESSAGE_TYPE, pos);

    result->addChild(params);

    return result;
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
    return AstNode::create(AST_BOOL, pos, "", strValue);
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
/// Creates an 'import' node.
/// </summary>
/// <param name="pos"></param>
/// <param name="value"></param>
/// <param name="flags"></param>
/// <returns></returns>
Ref<AstNode> astCreateImport(ScriptPosition pos, const std::string& value, int flags)
{
    return AstNode::create(AST_IMPORT, pos, "", value, flags);
}

/// <summary>
/// creates a node which obtains a pointer (the momeory address) of the child expression.
/// </summary>
/// <param name="pos"></param>
/// <param name="rExpr"></param>
/// <returns></returns>
Ref<AstNode> astCreateGetAddress(ScriptPosition pos, Ref<AstNode> rExpr)
{
    auto result = AstNode::create(AST_GET_ADDRESS, pos);
    result->addChild(rExpr);

    return result;
}

/// <summary>
/// Gathers all nodes referenced from the AST tree.
/// </summary>
/// <param name="root"></param>
/// <param name="nodes"></param>
static void astGatherAll(AstNode* root, set<AstNode*>& nodes)
{
    if (nodes.count(root) == 0)
    {
        nodes.insert(root);

        //Visit its data type.
        astGatherAll(root->getDataType(), nodes);

        //Visit children
        for (auto child : root->children())
        {
            if (child.notNull())
                astGatherAll(child.getPointer(), nodes);
        }
    }
}

//static void astGatherTypes(AstNode* root, set<AstNode*>& types)
//{
//    types.insert(root->getDataType());
//    for (auto child : root->children())
//    {
//        if (child.notNull())
//            astGatherTypes(child.getPointer(), types);
//    }
//}

/// <summary>
/// Gathers all types referenced from an AST tree.
/// Returns them in dependency order.
/// </summary>
/// <param name="root"></param>
/// <returns></returns>
std::vector<AstNode*> astGatherTypes(Ref<AstNode> root)
{
    set<AstNode*>	all;

    astGatherAll(root.getPointer(), all);

    vector<AstNode*> typesV;

    for (auto node : all)
    {
        if (astIsDataType(node))
            typesV.push_back(node);
    }

    return dependencySort<AstNode*>(typesV, [](AstNode* node) {
        set<AstNode*>	types;

        for (auto child : node->children())
        {
            if (child.notNull())
            {
                auto type = child->getDataType();
                if (!astIsVoidType(type))
                    types.insert(type);
            }
        }

        return types;
    });
}

/// <summary>
/// Gets all function nodes referenced from the AST
/// It cannot returned in reference order, because functions are allowed to
/// have circular references.
/// </summary>
/// <param name="root"></param>
/// <returns></returns>
std::vector<AstNode*> astGatherFunctions(AstNode* root)
{
    auto                all = astGatherAll(root);
    vector<AstNode*>    result;

    for (auto node : all)
    {
        if (node->getType() == AST_FUNCTION)
            result.push_back(node);
    }

    return result;
}

/// <summary>
/// Gathers all actors referenced from the tree.
/// Returns them in dependency order.
/// </summary>
std::vector<AstNode*> astGatherActors(AstNode* root)
{
    set<AstNode*>	all;

    astGatherAll(root, all);

    vector<AstNode*> actors;

    for (auto node : all)
    {
        if ( node->getType() == AST_ACTOR )
            actors.push_back(node);
    }

    return dependencySort<AstNode*>(actors, [](AstNode* actor) {
        set<AstNode*>	types;

        for (auto child : actor->children())
        {
            if (child.notNull() && child->getType() == AST_DECLARATION)
            {
                auto type = child->getDataType();
                if (type->getType() == AST_ACTOR)
                    types.insert(type);
            }
        }

        return types;
    });
}


/// <summary>
/// Gathers all nodes referenced from the AST tree.
/// </summary>
/// <param name="root"></param>
/// <returns></returns>
std::vector<AstNode*> astGatherAll(AstNode* root)
{
    set<AstNode*>   nodes;

    astGatherAll(root, nodes);
    return vector<AstNode*>(nodes.begin(), nodes.end());
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
        types[AST_FUNCTION_TYPE] = "AST_FUNCTION_TYPE";
        types[AST_ASSIGNMENT] = "AST_ASSIGNMENT";
        types[AST_FNCALL] = "AST_FNCALL";
        types[AST_CTCALL] = "AST_CTCALL";
        types[AST_INTEGER] = "AST_INTEGER";
        types[AST_FLOAT] = "AST_FLOAT";
        types[AST_STRING] = "AST_STRING";
        types[AST_BOOL] = "AST_BOOL";
        types[AST_IDENTIFIER] = "AST_IDENTIFIER";
        types[AST_ARRAY] = "AST_ARRAY";
        types[AST_MEMBER_ACCESS] = "AST_MEMBER_ACCESS";
        types[AST_MEMBER_NAME] = "AST_MEMBER_NAME";
        types[AST_BINARYOP] = "AST_BINARYOP";
        types[AST_PREFIXOP] = "AST_PREFIXOP";
        types[AST_POSTFIXOP] = "AST_POSTFIXOP";
        types[AST_ACTOR] = "AST_ACTOR";
        types[AST_DEFAULT_TYPE] = "AST_DEFAULT_TYPE";
        types[AST_TYPE_NAME] = "AST_TYPE_NAME";
        types[AST_INPUT] = "AST_INPUT";
        types[AST_MESSAGE_TYPE] = "AST_MESSAGE_TYPE";
        types[AST_OUTPUT] = "AST_OUTPUT";
        types[AST_UNNAMED_INPUT] = "AST_UNNAMED_INPUT";
        types[AST_IMPORT] = "AST_IMPORT";
        types[AST_GET_ADDRESS] = "AST_GET_ADDRESS";
        types[AST_ARRAY_DECL] = "AST_ARRAY_DECL";
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
        types["AST_FUNCTION_TYPE"] = AST_FUNCTION_TYPE;
        types["AST_ASSIGNMENT"] = AST_ASSIGNMENT;
        types["AST_FNCALL"] = AST_FNCALL;
        types["AST_CTCALL"] = AST_CTCALL;
        types["AST_INTEGER"] = AST_INTEGER;
        types["AST_FLOAT"] = AST_FLOAT;
        types["AST_STRING"] = AST_STRING;
        types["AST_BOOL"] = AST_BOOL;
        types["AST_IDENTIFIER"] = AST_IDENTIFIER;
        types["AST_ARRAY"] = AST_ARRAY;
        types["AST_MEMBER_ACCESS"] = AST_MEMBER_ACCESS;
        types["AST_MEMBER_NAME"] = AST_MEMBER_NAME;
        types["AST_BINARYOP"] = AST_BINARYOP;
        types["AST_PREFIXOP"] = AST_PREFIXOP;
        types["AST_POSTFIXOP"] = AST_POSTFIXOP;
        types["AST_ACTOR"] = AST_ACTOR;
        types["AST_DEFAULT_TYPE"] = AST_DEFAULT_TYPE;
        types["AST_TYPE_NAME"] = AST_TYPE_NAME;
        types["AST_INPUT"] = AST_INPUT;
        types["AST_MESSAGE_TYPE"] = AST_MESSAGE_TYPE;
        types["AST_OUTPUT"] = AST_OUTPUT;
        types["AST_UNNAMED_INPUT"] = AST_UNNAMED_INPUT;
        types["AST_IMPORT"] = AST_IMPORT;
        types["AST_GET_ADDRESS"] = AST_GET_ADDRESS;
        types["AST_ARRAY_DECL"] = AST_ARRAY_DECL;
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

/// <summary>Gets void data type</summary>
AstNode* astGetVoid()
{
    static auto node = astCreateTupleDef(ScriptPosition(), "");
    return node.getPointer();
}

// Gets bool default type.
AstNode* astGetBool()
{
    static auto node = AstNode::create(AST_DEFAULT_TYPE, ScriptPosition(), "bool");
    return node.getPointer();
}

//Gets int default type.
AstNode* astGetInt()
{
    static auto node = AstNode::create(AST_DEFAULT_TYPE, ScriptPosition(), "int");
    return node.getPointer();
}

//Gets 'C' pointer default type.
AstNode* astGetCPointer()
{
    static auto node = AstNode::create(AST_DEFAULT_TYPE, ScriptPosition(), "Cpointer");
    return node.getPointer();
}

//String representation of a tuple type, for debug purposes
static string astTupleTypeToString(AstNode* node)
{
    ostringstream	output;

    output << "(";
    const int count = node->childCount();

    for (int i = 0; i < count; ++i)
    {
        if (i > 0)
            output << ",";

        output << astTypeToString(node->child(i)->getDataType());
    }
    output << ")";

    return output.str();

}

//String representation of a function type, for debug purposes
static string astFunctionTypeToString(AstNode* node)
{
    string result = "function" + astTypeToString(astGetParameters(node));

    if (node->childExists(1))
        result += ":" + astTypeToString(astGetReturnType(node));

    return result;
}

/// <summary>
/// Gets a data type string representation, for debug purposes
/// </summary>
/// <param name="node"></param>
/// <returns></returns>
std::string astTypeToString(AstNode* node)
{
    switch (node->getType())
    {
    case AST_DEFAULT_TYPE:
        return node->getName();

    case AST_TUPLE:
    case AST_TUPLE_DEF:
        return astTupleTypeToString(node);

    case AST_FUNCTION:
    case AST_FUNCTION_TYPE:
        return astFunctionTypeToString(node);

    case AST_ACTOR:
        return string("actor '") + node->getName() + "'";

    case AST_INPUT:
        return "input" + astTypeToString(astGetParameters(node));
    case AST_MESSAGE_TYPE:
        return "message" + astTypeToString(astGetParameters(node));

    case AST_OUTPUT:
        return "output" + astTypeToString(astGetParameters(node));

    default:
        return "";
    }
}

/// <summary>
/// Gets the datatype of the parameters, for the types which have parameters.
/// </summary>
/// <param name="node"></param>
/// <returns>Parameters data type (should be a tuple) or void type if the node has no parameters</returns>
AstNode* astGetParameters(AstNode* node)
{
    switch (node->getType())
    {
    case AST_FUNCTION:
    case AST_FUNCTION_TYPE:
    case AST_ACTOR:
    case AST_INPUT:
    case AST_OUTPUT:
    case AST_MESSAGE_TYPE:
        return node->child(0)->getDataType();

    case AST_UNNAMED_INPUT:
        return node->child(1)->getDataType();

    case AST_TUPLE:
        return node->getDataType();

    case AST_TUPLE_DEF:
        return node;

    default:
        return astGetVoid();
    }
}

/// <summary>
/// Gets the return type of a data type, for the types which have it.
/// </summary>
/// <param name="node"></param>
/// <returns></returns>
AstNode* astGetReturnType(AstNode* node)
{
    switch (node->getType())
    {
    case AST_FUNCTION:
        return node->child(1)->getDataType();

    case AST_ACTOR:
        return node->getDataType();

    default:
        return astGetVoid();
    }
}

/// <summary>
/// Gets the function body node for several kinds of nodes.
/// </summary>
/// <param name="node"></param>
/// <returns></returns>
AstNode* astGetFunctionBody(AstNode* node)
{
    switch (node->getType())
    {
    case AST_FUNCTION:
    case AST_UNNAMED_INPUT:
        return node->child(2).getPointer();

    case AST_INPUT:
        return node->child(1).getPointer();

    default:
        assert(!"Not a function definition node.");
        return astGetVoid();
    }
}

/// <summary>
/// Checks if the node represents a tuple data type.
/// </summary>
/// <param name="node"></param>
/// <returns></returns>
bool astIsTupleType(const AstNode* node)
{
    auto t = node->getType();

    return t == AST_TUPLE_DEF; // || t == AST_TUPLE;
}

/// <summary>
/// Checks if the data type can be called (functions or messages, for example)
/// </summary>
/// <param name="node"></param>
/// <returns></returns>
bool astCanBeCalled(const AstNode* node)
{
    auto t = node->getType();

    return t == AST_FUNCTION 
        || t == AST_INPUT 
        || t == AST_OUTPUT
        || t == AST_MESSAGE_TYPE
        || t == AST_ACTOR;
}

/// <summary>Checks if a type is boolean</summary>
bool astIsBoolType(const AstNode* type)
{
    return type->getType() == AST_DEFAULT_TYPE && type->getName() == "bool";
}

/// <summary>Checks if a type is integer</summary>
bool astIsIntType(const AstNode* type)
{
    return type->getType() == AST_DEFAULT_TYPE && type->getName() == "int";
}

/// <summary>Checks if a type is a 'C' pointer</summary>
bool astIsCpointer(const AstNode* type)
{
    return type->getType() == AST_DEFAULT_TYPE && type->getName() == "Cpointer";
}


/// <summary>Checks if a type is boolean</summary>
bool astIsVoidType(const AstNode* type)
{
    return astIsTupleType(type) && type->childCount() == 0;
}

/// <summary>Checks if a node is a datatype</summary>
bool astIsDataType(const AstNode* node)
{
    switch (node->getType())
    {
    case AST_TUPLE_DEF:
    //case AST_TUPLE:
    case AST_ACTOR:
    case AST_FUNCTION_TYPE:
    case AST_MESSAGE_TYPE:
    case AST_ARRAY_DECL:
        return true;
    default:
        return false;
    }
}


/// <summary>
/// Finds a child node by its name
/// </summary>
/// <param name="node"></param>
/// <param name="name"></param>
/// <returns>Child index or -1 if does not find it.</returns>
int astFindMemberByName(AstNode* node, const std::string& name)
{
    for (size_t i = 0; i < node->childCount(); ++i)
    {
        if (node->child(i)->getName() == name)
            return i;
    }

    return -1;
}
