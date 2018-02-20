/// <summary>
/// Abstract Syntax Tree classes / functions
/// </summary>

#include "pch.h"
#include "ast.h"
#include "SymbolScope.h"
#include "dataTypes.h"

using namespace std;

//Empty children list constant
const AstNodeList AstNode::ms_noChildren;

//Global AST node count.
int AstNode::ms_nodeCount = 0;


AstNode::AstNode(AstNodeTypes type, const ScriptPosition& pos) :
	m_position(pos), m_type(type)
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


//  Constructor functions.
//
////////////////////////////////
Ref<AstNode> astCreateScript(ScriptPosition pos)
{
    return refFromNew( new AstBranchNode(AST_SCRIPT, pos));    
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
	auto result = refFromNew(
		new AstNamedBranch(AST_DECLARATION, pos, name)
	);

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
	auto result = refFromNew(
		new AstNamedBranch(AST_TYPEDEF, pos, name)
	);

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
	auto result = refFromNew(new AstNamedBranch(AST_FUNCTION, pos, name));

	result->addChild(params);
	result->addChild(returnType);
	result->addChild(bodyExpr);

	return result;
}


Ref<AstNode> astCreateBlock(LexToken token)
{
	return refFromNew(new AstBranchNode(AST_BLOCK, token.getPosition()));
}

Ref<AstNode> astCreateTuple(LexToken token)
{
	return refFromNew(new AstBranchNode(AST_TUPLE, token.getPosition()));
}

Ref<AstNode> astCreateTupleDef(ScriptPosition pos, const std::string& name)
{
	return refFromNew(new AstNamedBranch(AST_TUPLE_DEF, pos, name));
}

Ref<AstNode> astCreateTupleAdapter(Ref<AstNode> tupleNode)
{
	auto result = refFromNew(new AstBranchNode(AST_TUPLE_ADAPTER, tupleNode->position()));

	result->addChild(tupleNode);
	return result;
}


Ref<AstNode> astCreateIf (ScriptPosition pos, 
                          Ref<AstNode> condition,
                          Ref<AstNode> thenSt,
                          Ref<AstNode> elseSt)
{
    
    auto result = refFromNew( new AstBranchNode(AST_IF, pos));
    
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
    auto result = refFromNew( new AstBranchNode(AST_FOR, pos));
    
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
    auto result = refFromNew( new AstBranchNode(AST_FOR_EACH, pos));
    
    result->addChild(itemDeclaration);
    result->addChild(sequenceExpr);
    result->addChild(body);

    return result;
}


Ref<AstNode> astCreateReturn (ScriptPosition pos, Ref<AstNode> expr)
{
    auto result = refFromNew( new AstBranchNode(AST_RETURN, pos));
    
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
    auto result = refFromNew( 
		new AstOperator(AST_ASSIGNMENT, opToken.getPosition(), opToken.text())
	);
    
    result->addChild(lexpr);
    result->addChild(rexpr);
    return result;
}

Ref<AstNode> astCreatePrefixOp(LexToken token, Ref<AstNode> rexpr)
{
    auto result = refFromNew( new AstOperator(AST_PREFIXOP, 
                                              token.getPosition(),
                                              token.text()));
    
    result->addChild(rexpr);
    return result;
}

Ref<AstNode> astCreatePostfixOp(LexToken token, Ref<AstNode> lexpr)
{
    auto result = refFromNew( new AstOperator(AST_POSTFIXOP, 
                                              token.getPosition(),
                                              token.text()));
    
    result->addChild(lexpr);
    return result;
}

Ref<AstNode> astCreateBinaryOp(LexToken token, 
                                 Ref<AstNode> lexpr, 
                                 Ref<AstNode> rexpr)
{
    auto result = refFromNew( new AstOperator(AST_BINARYOP, 
                                              token.getPosition(),
                                              token.text()));
    
    result->addChild(lexpr);
    result->addChild(rexpr);
    return result;
}

Ref<AstNode> astCreateFnCall(ScriptPosition pos, Ref<AstNode> fnExpr, Ref<AstNode> params)
{
    auto result = refFromNew( new AstBranchNode(AST_FNCALL, pos));
    
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
    return refFromNew( new AstBranchNode(AST_ARRAY, pos));
}

Ref<AstNode> astCreateArrayAccess(ScriptPosition pos,
                                  Ref<AstNode> arrayExpr, 
                                  Ref<AstNode> indexExpr)
{    
    auto result = refFromNew( new AstBranchNode(AST_ARRAY_ACCESS, pos));
    
    result->addChild(arrayExpr);
    result->addChild(indexExpr);

    return result;
}

Ref<AstNode> astCreateMemberAccess(ScriptPosition pos,
                                  Ref<AstNode> objExpr, 
                                  Ref<AstNode> identifier)
{    
    auto result = refFromNew( new AstBranchNode(AST_MEMBER_ACCESS, pos));
    
    result->addChild(objExpr);
    result->addChild(identifier);

    return result;
}

Ref<AstNode> astCreateActor(ScriptPosition pos, const std::string& name)
{
    return refFromNew(new AstNamedBranch(AST_ACTOR, pos, name));
}

Ref<AstNode> astCreateInputMsg(ScriptPosition pos, const std::string& name)
{
    return refFromNew(new AstNamedBranch(AST_INPUT, pos, name));
}

Ref<AstNode> astCreateOutputMsg(ScriptPosition pos, const std::string& name)
{
    return refFromNew(new AstNamedBranch(AST_OUTPUT, pos, name));
}

//Ref<AstNode> astCreateConnect(ScriptPosition pos,
//                                 Ref<AstNode> lexpr, 
//                                 Ref<AstNode> rexpr)
//{
//    auto result = refFromNew (new AstOperator(AST_CONNECT, pos, LEX_CONNECT));
//    
//    result->addChild(lexpr);
//    result->addChild(rexpr);
//    return result;
//}
//
//Ref<AstNode> astCreateSend(ScriptPosition pos,
//                            Ref<AstNode> lexpr, 
//                            Ref<AstNode> rexpr)
//{
//    auto result = refFromNew (new AstOperator(AST_BINARYOP, pos, LEX_SEND));
//    
//    result->addChild(lexpr);
//    result->addChild(rexpr);
//    return result;
//}
//
//Ref<AstNode> astCreateExtends (ScriptPosition pos,
//                                const std::string& parentName)
//{
//    return refFromNew (new AstNamedBranch(AST_EXTENDS, pos, parentName));
//}
//
//Ref<AstNode> astCreateExport (ScriptPosition pos, Ref<AstNode> child)
//{
//    auto result = refFromNew(new AstBranchNode(AST_EXPORT, pos));
//    
//    result->addChild(child);
//    return result;
//}
//
//Ref<AstNode> astCreateImport (ScriptPosition pos, Ref<AstNode> param)
//{
//    auto result = refFromNew(new AstBranchNode(AST_IMPORT, pos));
//    
//    result->addChild(param);
//    return result;
//}
//
///**
// * Gets the 'extends' node of a class node.
// * @param node
// * @return 
// */
//Ref<AstNode> astGetExtends(Ref<AstNode> node)
//{
//    ASSERT (node->getType() == AST_CLASS);
//    if (!node->childExists(0))
//        return Ref<AstNode>();
//    
//    auto child = node->children().front();
//    if (child->getType() != AST_EXTENDS)
//        return Ref<AstNode>();
//    else
//        return child;    
//}
//
/**
 * Creates an 'AstLiteral' object from a source token.
 * @param token
 * @return 
 */
Ref<AstLiteral> AstLiteral::create(LexToken token)
{
	Ref<AstLiteral> result;
	auto			pos = token.getPosition();
    
    switch (token.type())
    {
    case LEX_STR:

		result = refFromNew(new AstLiteral(pos, AST_STRING));
		result->m_strValue = token.strValue();
		return result;

    case LEX_INT:        
		result = refFromNew(new AstLiteral(pos, AST_INTEGER));
		break;

	case LEX_FLOAT:
		result = refFromNew(new AstLiteral(pos, AST_FLOAT));
		break;
        
    default:
        assert(!"Invalid token for a literal");
    }
    
	result->m_strValue = token.text();
	return result;
}

/// <summary>
/// Creates an 'AStLiteral' from a boolean value.
/// </summary>
/// <param name="pos"></param>
/// <param name="value"></param>
/// <returns></returns>
Ref<AstLiteral> AstLiteral::createBool(ScriptPosition pos, bool value)
{
	auto result = refFromNew(new AstLiteral(pos, AST_BOOL));

	result->m_strValue = value ? "1" : "0";
	return result;
}


///**
// * Creates a literal from an integer
// * @param pos
// * @param value
// * @return 
// */
//Ref<AstLiteral> AstLiteral::create(ScriptPosition pos, int value)
//{
//    return refFromNew(new AstLiteral(pos, jsInt(value)));
//}
//
///**
// * Creates a 'null' literal
// * @param pos
// * @return 
// */
//Ref<AstLiteral> AstLiteral::createNull(ScriptPosition pos)
//{
//    return refFromNew(new AstLiteral(pos, jsNull()));
//}
//
///**
// * Gets the 'extends' node of a class. The 'extends' node contains inheritance 
// * information.
// * @return The node or a NULL pointer if not present.
// */
//Ref<AstNamedBranch> AstClassNode::getExtendsNode()const
//{
//    if (this->childExists(0))
//    {
//        auto child = this->children().front();
//        
//        if (child->getType() == AST_EXTENDS)
//            return child.staticCast<AstNamedBranch>();
//    }
//    
//    return Ref<AstNamedBranch>();
//}
//
///**
// * Transforms an AST statement into a Javascript object. 
// * This particular version creates an object containing all its children
// * @return 
// */
//ASValue AstNode::toJS()const
//{
//    Ref<JSObject>   obj = JSObject::create();
//    
//    obj->writeField("a_type", jsString(astTypeToString(getType())), false);
//
//    const string name = getName();
//    if (!name.empty())
//        obj->writeField("b_name", jsString(name), false);
//    
//    const AstNodeList&  c = children();
//    if (!c.empty())
//        obj->writeField("z_children", toJSArray(c)->value(), false);
//
//    const auto value = getValue();
//    if (!value.isNull())
//        obj->writeField("v_value", value, false);
//
//    return obj->value();
//}
//
//
///**
// * Function declaration to JSValue
// * @return 
// */
//ASValue AstFunction::toJS()const
//{
//    Ref<JSObject>   obj = AstNode::toJS().staticCast<JSObject>();
//    
//    obj->writeField("c_parameters", JSArray::createStrArray(m_params)->value(), false);
//    if (m_code.notNull())
//        obj->writeField("d_code", m_code->toJS(), false);
//    
//    return obj->value();
//}
//
///**
// * Class node to javascript object
// * @return 
// */
//ASValue AstClassNode::toJS()const
//{
//    Ref<JSObject>   obj = AstNamedBranch::toJS().staticCast<JSObject>();
//    
//    obj->writeField("c_parameters", JSArray::createStrArray(m_params)->value(), false);
//    
//    return obj->value();
//}
//
///**
// * Actor node to javascript object
// * @return 
// */
//ASValue AstActor::toJS()const
//{
//    Ref<JSObject>   obj = AstNamedBranch::toJS().staticCast<JSObject>();
//    
//    obj->writeField("c_parameters", JSArray::createStrArray(m_params)->value(), false);
//    
//    return obj->value();
//}
//
///**
// * Operator to JSValue
// * @return 
// */
//ASValue AstOperator::toJS()const
//{
//    Ref<JSObject>   obj = AstBranchNode::toJS().staticCast<JSObject>();
//
//    obj->writeField("d_operator", jsString(getTokenStr(code)), false);
//    return obj->value();
//}
//
///**
// * Object literal to JSValue
// * @return 
// */
//ASValue AstObject::toJS()const
//{
//    Ref<JSObject>   obj = JSObject::create();
//    Ref<JSObject>   props = JSObject::create();
//    
//    obj->writeField("a_type", jsString(astTypeToString(getType())), false);
//    obj->writeField("b_properties", props->value(), false);
//    
//    PropertyList::const_iterator it;
//    for (it = m_properties.begin(); it != m_properties.end(); ++it)
//        props->writeField(it->name, it->expr->toJS(), false);
//    
//    return obj->value();
//}
//
///**
// * Transforms a list of AstNodes into a Javascript Array.
// * @param statements
// * @return 
// */
//Ref<JSArray> toJSArray (const AstNodeList& statements)
//{
//    Ref<JSArray>    result = JSArray::create();
//    
//    for (size_t i = 0; i < statements.size(); ++i)
//    {
//        if (statements[i].notNull())
//            result->push( statements[i]->toJS() );
//        else
//            result->push(jsNull());
//    }
//    
//    return result;
//}
//

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
		types[AST_CONNECT] = "AST_CONNECT";
		types[AST_DEFAULT_TYPE] = "AST_DEFAULT_TYPE";
		types[AST_TYPE_NAME] = "AST_TYPE_NAME";
		types[AST_INPUT] = "AST_INPUT";
		types[AST_OUTPUT] = "AST_OUTPUT";
		//types[AST_TYPES_COUNT] = "AST_TYPES_COUNT";

		assert(types.size() == AST_TYPES_COUNT);
    }
    
    TypesMap::const_iterator it = types.find(type);
    
	if (it != types.end())
		return it->second;
	else if (type > AST_TYPES_COUNT)
		return "BAD_AST_TYPE";
	else
		return "AST type number: " + to_string(type);
}

/// <summary>
/// Creates an AST node for a default (predefined) data type.
/// </summary>
/// <returns></returns>
Ref<AstNode> astCreateDefaultType(Ref<DefaultType> type)
{
	auto node = refFromNew(new AstNamedBranch(AST_DEFAULT_TYPE, ScriptPosition(), type->getName()));
	node->setDataType(type);
	return node;
}
