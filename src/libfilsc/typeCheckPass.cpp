#include "pch.h"
#include "typeCheckPass.h"
#include "semanticAnalysis_internal.h"
#include "symbolScope.h"
#include "passOperations.h"
#include "semAnalysisState.h"

using namespace std;

/// <summary>
/// This pass checks that type references are valid.
/// </summary>
/// <param name="node"></param>
/// <param name="state"></param>
/// <returns></returns>
SemanticResult typeCheckPass(Ref<AstNode> node, SemAnalysisState& state)
{
    static PassOperations	functions;

    if (functions.empty())
    {
        functions.add(AST_TYPE_NAME, typeExistsCheck);
        functions.add(AST_TUPLE_DEF, tupleDefTypeCheck);

        functions.add(AST_BLOCK, blockTypeCheck);
        functions.add(AST_TYPEDEF, typedefTypeCheck);
        functions.add(AST_TUPLE, tupleTypeCheck);
        functions.add(AST_DECLARATION, declarationTypeCheck);
        functions.add(AST_IF, ifTypeCheck);
        functions.add(AST_RETURN, returnTypeAssign);
        functions.add(AST_FUNCTION, functionDefTypeCheck);
        functions.add(AST_FUNCTION_TYPE, assignItselftAsType);
        functions.add(AST_ASSIGNMENT, assignmentTypeCheck);
        functions.add(AST_FNCALL, callTypeCheck);
        functions.add(AST_CTCALL, compileTimeCallTypeCheck);        
        functions.add(AST_INTEGER, literalTypeAssign);
        functions.add(AST_FLOAT, literalTypeAssign);
        functions.add(AST_STRING, literalTypeAssign);
        functions.add(AST_BOOL, literalTypeAssign);
        functions.add(AST_IDENTIFIER, varReadTypeCheck);
        functions.add(AST_MEMBER_ACCESS, memberAccessTypeCheck);
        functions.add(AST_BINARYOP, binaryOpTypeCheck);
        functions.add(AST_PREFIXOP, prefixOpTypeCheck);
        functions.add(AST_POSTFIXOP, postfixOpTypeCheck);
        functions.add(AST_ACTOR, actorTypeCheck);
        functions.add(AST_INPUT, messageTypeCheck);
        functions.add(AST_MESSAGE_TYPE, assignItselftAsType);
        functions.add(AST_OUTPUT, messageTypeCheck);
        functions.add(AST_UNNAMED_INPUT, unnamedInputTypeCheck);
        functions.add(AST_ARRAY_DECL, arrayDeclarationTypeCheck);
        functions.add(AST_MODULE, moduleTypeCheck);

        functions.add(AST_ASSIGNMENT, addTupleAdapter);
    }

    return semInOrderWalk(functions, state, node);
}

/// <summary>
/// Second phase of type check.
/// They are operations that require the first type check phase (pass) to be complete.
/// </summary>
/// <param name="node"></param>
/// <param name="state"></param>
/// <returns></returns>
SemanticResult typeCheckPass2(Ref<AstNode> node, SemAnalysisState& state)
{
    static PassOperations	functions;

    if (functions.empty())
    {
        functions.add(AST_RETURN, returnTypeCheck);
        functions.add(AST_RETURN, addReturnTupleAdapter);
    }

    return semInOrderWalk(functions, state, node);

}

/// <summary>
/// Performs the operations that are needed prior to type check
/// </summary>
/// <param name="node"></param>
/// <param name="state"></param>
/// <returns></returns>
SemanticResult preTypeCheckPass(Ref<AstNode> node, SemAnalysisState& state)
{
    static PassOperations	functions;

    if (functions.empty())
    {
        functions.add(AST_IDENTIFIER, recursiveSymbolReferenceCheck);

        functions.add(AST_TYPEDEF, tupleRemoveTypedef);
    }

    return semInOrderWalk(functions, state, node);
}


/// <summary>
/// Checks that the referenced symbol is not referenced in its initialization expression
/// </summary>
CompileError recursiveSymbolReferenceCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    //auto referenced = node->getScope()->get(node->getName(), false);
    auto referenced = state.getScope(node)->get(node->getName(), false);
    //auto referenced = node->getReference();
    int i = 0;

    for (; state.parent(i).notNull(); ++i)
    {
        if (referenced == state.parent(i))
            return semError(node, ETYPE_RECURSIVE_SYMBOL_REFERENCE_1, node->getName().c_str());
    }

    return CompileError::ok();
}


/// <summary>
/// Checks that the referenced type exists, and assign the type to the node.
/// </summary>
CompileError typeExistsCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    auto	scope = state.getScope(node);
    string	name = node->getName();

    auto typeNode = scope->get(name, true);

    if (typeNode.isNull())
        return semError(node, ETYPE_NON_EXISTENT_SYMBOL_1, name.c_str());

    if (!isType(typeNode))
        return semError(node, ETYPE_NOT_A_TYPE_1, name.c_str());
    else
        node->setDataType(typeNode->getDataType());

    return CompileError::ok();
}

/// <summary>Type checking on a tuple definition.</summary>
CompileError tupleDefTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    //The data type of a tuple definition is itself.
    return assignItselftAsType(node, state);
}

/// <summary>
/// Performs type checking on a block node.
/// </summary>
CompileError blockTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    if (node->childCount() == 0)
        return setVoidType(node, state);
    else
    {
        node->setDataType(node->children().back()->getDataType());
        return CompileError::ok();
    }
}

/// <summary>
/// Type check for a 'typedef' declaration.
/// Just propagates the refenced type. 
/// </summary>
CompileError typedefTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    node->setDataType(node->child(0)->getDataType());
    return CompileError::ok();
}

/// <summary>
/// Type check on a tuple creation node. Builds an unnamed tuple definition as type.
/// </summary>
CompileError tupleTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    auto tupleType = astCreateTupleDef(node->position(), "");

    for (auto child : node->children())
    {
        assert(child.notNull());

        auto decl = astCreateDeclaration(child->position(), "", child->getDataType(), child);
        decl->addFlag(ASTF_CONST);
        decl->setDataType(child->getDataType());

        tupleType->addChild(decl);
    }

    tupleType = state.registerUnnamedType(tupleType);

    node->setDataType(tupleType.getPointer());

    return CompileError::ok();
}

/// <summary>
/// Declaration type check
/// </summary>
CompileError declarationTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    //Type declared explicitly?
    if (node->childExists(0))
    {
        auto declaredType = node->child(0)->getDataType();

        if (declaredType->getType() == AST_ACTOR)
            return actorInstanceTypeCheck(node, state);

        node->setDataType(declaredType);

        if (!node->childExists(1))
            return CompileError::ok();	//No initialization
        else
            return assignCheck(declaredType, node, 1);
    }
    else
    {
        if (!node->childExists(1))
            return semError(node, ETYPE_DECLARATION_WITHOUT_TYPE);
        else
        {
            //Infer type from initialization expression.
            node->setDataType(node->child(1)->getDataType());
            return CompileError::ok();
        }
    }
}

/// <summary>Type checking for 'if' expressions</summary>
CompileError ifTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    auto conditionType = node->child(0)->getDataType();

    if (!astIsBoolType(conditionType))
    {
        return semError(node->child(0),
            ETYPE_WRONG_IF_CONDITION_TYPE_1,
            astTypeToString(conditionType).c_str());
    }

    auto thenType = node->child(1)->getDataType();

    if (!node->childExists(2))
        setVoidType(node, state);
    else
    {
        auto elseType = node->child(2)->getDataType();
        auto common = getCommonType(thenType, elseType, state);

        if (common != nullptr)
            node->setDataType(common);
        else
        {
            //It is not an error to have incompatible types on both branches.
            //But in this case, the 'if' expression takes 'void' type.
            setVoidType(node, state);
        }
    }

    return CompileError::ok();
}

/// <summary>'return' expressions type check</summary>
/// <remarks>Just assign the type, as at the moment in which is called
/// the function return type has not been assigned.
/// </remarks>
CompileError returnTypeAssign(Ref<AstNode> node, SemAnalysisState& state)
{
    if (node->childExists(0))
    {
        node->setDataType(node->child(0)->getDataType());
        return CompileError::ok();
    }
    else
        return setVoidType(node, state);
}

/// <summary>
/// Check that the return statement expression type matches the return type of 
/// the function which contains it.
/// It also tests that the return statement belongs to a function.
/// </summary>
CompileError returnTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    auto functionNode = state.findParent([](auto node) {
        return node->getType() == AST_FUNCTION;
    });

    if (functionNode.isNull())
        return semError(node, ETYPE_RETURN_OUTSIDE_FUNCTION);

    auto returnType = astGetReturnType(functionNode.getPointer());
    bool ok = false;

    if (!node->childExists(0))
    {
        //Empty return 
        ok = astIsVoidType(returnType);
    }
    else
    {
        ok = assignCheck(returnType, node, 0).isOk();
    }

    if(!ok)
    {
        return semError(node, ETYPE_INCOMPATIBLE_RETURN_TYPE_2,
            astTypeToString(node.getPointer()).c_str(),
            astTypeToString(returnType).c_str());
    }
    else
        return CompileError::ok();
}


/// <summary>
/// Performs type checking on a function definition.
/// </summary>
CompileError functionDefTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    auto body = node->child(2);
    auto bodyType = astGetVoid();
    
    if (body.notNull())
        bodyType = body->getDataType();

    //The data type of the function is itself
    //TODO: An 'AST_FUNCTION_TYPE' node should be created instead.
    node->setDataType(node.getPointer());

    //Infer return type if necessary.
    if (!node->childExists(1))
    {
        node->setChild(1, ref(body->getDataType()));
        return CompileError::ok();
    }
    else
    {
        auto declaredType = node->child(1)->getDataType();

        //If void is declared explicitly, then any type is valid for the body.
        if (astIsVoidType(declaredType))
            return CompileError::ok();
        
        //If the function has no body, no type check is necessary.
        if (body.isNull())
            return CompileError::ok();
        else
            return assignCheck(declaredType, node, 2);
    }
}


/// <summary>Type checking for assignment operations.</summary>
CompileError assignmentTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    auto leftType = node->child(0)->getDataType();

    node->setDataType(leftType);
    return assignCheck (leftType, node, 1);
}

/// <summary>Performs type checking for function calls.</summary>
CompileError callTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    auto type = node->child(0)->getDataType();

    if (!astCanBeCalled(type))
        return semError(node, ETYPE_NOT_CALLABLE);

    node->setDataType(astGetReturnType(type));
    return assignCheck(astGetParameters(type), node, 1);
}

/// <summary>Type checking for 'compile time' call operator (fn[...]),
/// which is also used to access array elements (for historical reasons)
/// </summary>
CompileError compileTimeCallTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    auto fnExpr = node->child(0);

    switch (fnExpr->getDataType()->getType())
    {
    case AST_ARRAY_DECL:
        return arrayAccessTypeCheck(node, state);

    case AST_TUPLE_DEF:
        return tupleItemAccessTypeCheck(node, state);

    default:
        return semError(node, ETYPE_NOT_IMPLEMENTED_1, "Compile time parameters evaluation");
    }
}

/// <summary>
/// Type checks the array access operator.
/// </summary>
CompileError arrayAccessTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    auto arrayExpr = node->child(0);
    auto params = node->child(1);

    if (params->childCount() != 1)
        return semError(params, ETYPE_INVALID_ARRAY_INDEX);

    auto indexExpr = params->child(0);
    if (!astIsIntType (indexExpr->getDataType()))
        return semError(params, ETYPE_INVALID_ARRAY_INDEX);

    //The type of the expression is the type of the array items
    auto dataType = arrayExpr->getDataType()->child(0)->getDataType();
    node->setDataType(dataType);

    return CompileError::ok();
}

/// <summary>Type checks a tuple item access operation.</summary>
CompileError tupleItemAccessTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    auto tupleExpr = node->child(0);
    auto tupleType = tupleExpr->getDataType();
    auto params = node->child(1);

    if (params->childCount() != 1)
        return semError(params, ETYPE_INVALID_TUPLE_INDEX);

    auto indexExpr = params->child(0);
    if (indexExpr->getType() != AST_INTEGER)
        return semError(params, ETYPE_INVALID_TUPLE_INDEX);

    int indexVal = stoi(indexExpr->getValue());
    const int limit = (int)tupleType->childCount();

    //Range check (can be done at compile time)
    if (indexVal < 0 || indexVal >= limit)
        return semError(params, ETYPE_TUPLE_INDEX_OUT_OF_RANGE_2, indexVal, limit);

    //The type of the expression is the type of the tuple item.
    auto dataType = tupleType->child(indexVal)->getDataType();
    node->setDataType(dataType);

    return CompileError::ok();
}

/// <summary>Type check for variable / symbol reading</summary>
/// <param name="node"></param>
/// <param name="state"></param>
/// <returns></returns>
CompileError varReadTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    auto scope = state.getScope(node);

    auto referenced = scope->get(node->getName(), true);

    if (referenced.isNull())
        return semError(node, ETYPE_NON_EXISTENT_SYMBOL_1, node->getName().c_str());
    else
    {
        //TODO: If the referenced node has not been type-checked at this moment, 
        //the type of the reference node may be still void when this function returns.
        node->setReference(referenced.getPointer());
        return CompileError::ok();
    }
}

/// <summary>Type checking of member access operations.</summary>
CompileError memberAccessTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    auto	leftType = node->child(0)->getDataType();
    assert(astIsTupleType(leftType));

    string	name = node->child(1)->getName();

    int index = astFindMemberByName(leftType, name);

    if (index < 0)
    {
        return semError(node->child(1),
            ETYPE_MEMBER_NOT_FOUND_2,
            name.c_str(),
            astTypeToString(leftType).c_str());
    }
    else
    {
        auto memberType = leftType->child(index)->getDataType();
        node->setDataType(memberType);
        return CompileError::ok();
    }
}

/// <summary>Type check for binary operators</summary>
CompileError binaryOpTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    string op = node->getValue();

    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%")
        return mathOperatorTypeCheck(node, state);
    else if (op == ">>" || op == "<<" || op == "&" || op == "|" || op == "^")
        return bitwiseOperatorTypeCheck(node, state);
    else if (op == "<" || op == ">" || op == ">=" || op == "<=")
        return comparisionOperatorTypeCheck(node, state);
    else if (op == "==" || op == "!=")
        return equalityOperatorTypeCheck(node, state);
    else
        return logicalOperatorTypeCheck(node, state);
}

/// <summary>Type check for prefix operators</summary>
CompileError prefixOpTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    string op = node->getValue();
    auto childType = node->child(0)->getDataType();

    node->setDataType(childType);

    if (op == "!")
    {
        if (!astIsBoolType(childType))
            return semError(node->child(0), ETYPE_WRONG_TYPE_2, astTypeToString(childType).c_str(), "bool");
    }
    else
    {
        if (!astIsIntType(childType))
            return semError(node->child(0), ETYPE_WRONG_TYPE_2, astTypeToString(childType).c_str(), "int");
    }

    return CompileError::ok();
}

/// <summary>Type check for postfix operators</summary>
CompileError postfixOpTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    auto childType = node->child(0)->getDataType();

    node->setDataType(childType);

    if (!astIsIntType(childType))
        return semError(node->child(0), ETYPE_WRONG_TYPE_2, astTypeToString(childType).c_str(), "int");
    else
        return CompileError::ok();
}

/// <summary>Type check for arithmetic operators</summary>
CompileError mathOperatorTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    //TODO: Support other types different from integers.
    auto lexpr = node->child(0);
    auto rexpr = node->child(1);

    node->setDataType(lexpr->getDataType());

    if (!astIsIntType(lexpr->getDataType()))
        return semError(node->child(0), ETYPE_WRONG_TYPE_2, astTypeToString(lexpr->getDataType()).c_str(), "int");
    else if (!astIsIntType(rexpr->getDataType()))
        return semError(node->child(0), ETYPE_WRONG_TYPE_2, astTypeToString(rexpr->getDataType()).c_str(), "int");
    else
        return CompileError::ok();
}

/// <summary>Type check for bitwise operators</summary>
CompileError bitwiseOperatorTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    //It is the same, by the moment.
    return mathOperatorTypeCheck(node, state);
}

/// <summary>Type check for comparision operators</summary>
CompileError comparisionOperatorTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    //Also same checks as in math operators, but the type is boolean.
    auto result = mathOperatorTypeCheck(node, state);

    node->setDataType(astGetBool());
    return result;
}

/// <summary>Type check for equality and inequality operators</summary>
CompileError equalityOperatorTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    //TODO: Support other types different from integers and booleans.
    auto lexpr = node->child(0);
    auto rexpr = node->child(1);

    node->setDataType(astGetBool());

    if (astIsIntType(lexpr->getDataType()))
    {
        if (!astIsIntType(rexpr->getDataType()))
            return semError(rexpr, ETYPE_WRONG_TYPE_2, astTypeToString(rexpr->getDataType()).c_str(), "int");
    }
    else if (astIsBoolType(lexpr->getDataType()))
    {
        if (!astIsBoolType(rexpr->getDataType()))
            return semError(rexpr, ETYPE_WRONG_TYPE_2, astTypeToString(rexpr->getDataType()).c_str(), "bool");
    }
    else
        return semError(lexpr, ETYPE_WRONG_TYPE_2, astTypeToString(rexpr->getDataType()).c_str(), "int or bool");

    //If ir reach here, check is ok.
    return CompileError::ok();
}

/// <summary>Type check for logical operators</summary>
CompileError logicalOperatorTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    auto lexpr = node->child(0);
    auto rexpr = node->child(1);

    node->setDataType(lexpr->getDataType());

    if (!astIsBoolType(lexpr->getDataType()))
        return semError(node->child(0), ETYPE_WRONG_TYPE_2, astTypeToString(lexpr->getDataType()).c_str(), "bool");
    else if (!astIsBoolType(rexpr->getDataType()))
        return semError(node->child(0), ETYPE_WRONG_TYPE_2, astTypeToString(rexpr->getDataType()).c_str(), "bool");
    else
        return CompileError::ok();
}

/// <summary>Assigns the type for a literal expression.</summary>
CompileError literalTypeAssign(Ref<AstNode> node, SemAnalysisState& state)
{
    switch (node->getType())
    {
    case AST_BOOL:
        node->setDataType(astGetBool());
        break;

    case AST_INTEGER:
        node->setDataType(astGetInt());
        break;

    case AST_STRING:
        return semError(node, ETYPE_NOT_IMPLEMENTED_1, "String literals");
        break;

    default:
        assert(!"Unexpected literal type");
    }

    return CompileError::ok();
}

/// <summary>
/// Performs type-checking for actor declarations.
/// </summary>
CompileError actorTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    //The data type of an actor is itself
    return assignItselftAsType(node, state);
}

/// <summary>
/// For nodes which are themselves a type.
/// </summary>
CompileError assignItselftAsType(Ref<AstNode> node, SemAnalysisState& state)
{
    node->setDataType(node.getPointer());

    return CompileError::ok();
}

/// /// <summary>
/// Performs type checking for actor messages.
/// </summary>
CompileError messageTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    //Create a type from the input message definition.
    auto msgType = astCreateMessageType(node->position(), astGetParameters(node.getPointer()));

    //It is added as a child, to link is lifetime to the message.
    node->addChild(msgType);
    node->setDataType(msgType.getPointer());

    return CompileError::ok();
}

/// <summary>
/// Type checking for unnnamed input redirections.
/// </summary>
CompileError unnamedInputTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    auto pathNode = node->child(0);

    if (pathNode->childCount() == 0)
        return semError(node, ETYPE_UNSPECIFIED_CONNECT_OUTPUT);

    auto output = getConnectOutputType(pathNode, state);
    if (output == nullptr)
        return semError(node, ETYPE_INVALID_CONNECT_OUTPUT);

    return assignCheck (astGetParameters(output), node, 1);
}


/// <summary>Type checking of an actor instance.</summary>
CompileError actorInstanceTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    //TODO: Ensure this function is tested.
    assert(node->getType() == AST_DECLARATION);
    assert(node->childExists(0));

    auto actorType = node->child(0)->getDataType();
    auto parent = state.parent();

    if (parent->getType() != AST_ACTOR)
        return semError(node, ETYPE_MISPLACED_ACTOR_INSTANCE);

    if (!node->hasFlag(ASTF_CONST))
        return semError(node, ETYPE_NON_CONST_ACTOR_INSTANCE);

    node->setDataType(actorType);

    if (node->childExists(1))
    {
        //Call with parameters
        return assignCheck(actorType, node, 1);
    }
    else
    {
        //TODO: Support default parameters.
        //TODO: More explicit error message.
        auto params = astGetParameters(actorType);
        if (astIsVoidType(params))
            return CompileError::ok();
        else
            return incompatibleTypesError(params, astGetVoid()).errors[0];
    }
}

/// <summary> Type checking for array declarations.</summary>
CompileError arrayDeclarationTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    auto typeSpec = node->child(0);
    auto sizeExpr = node->child(1);

    if (sizeExpr->getType() != AST_INTEGER)
        return semError(sizeExpr, ETYPE_INVALID_ARRAY_SIZE);

    //Data type its itself.
    node->setDataType(node.getPointer());
    return CompileError::ok();
}

///<summary> Module type check</summary>
CompileError moduleTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
    //Just mark it as 'type checked'
    node->addFlag(ASTF_TYPECHECKED);
    return CompileError::ok();
}

/// <summary>
/// Gets the data type of the refered output in a connect expression, taking the 'path'
/// node as input.
/// If not found or not the expected data type, it would return a null pointer.
/// </summary>
/// <param name="pathNode"></param>
/// <param name="state"></param>
/// <returns></returns>
AstNode* getConnectOutputType(Ref<AstNode> pathNode, SemAnalysisState& state)
{
    auto			scope = state.getScope(pathNode);

    auto referred = scope->get(pathNode->child(0)->getName(), true);
    if (referred.isNull())
        return nullptr;

    auto result = referred->getDataType();
    auto actor = result;

    size_t i = 1;
    for (; i < pathNode->childCount() && actor->getType() == AST_ACTOR; ++i)
    {
        auto child = pathNode->child(i);
        assert(child.notNull() && child->getType() == AST_MEMBER_NAME);

        int index = astFindMemberByName(actor, child->getName());

        if (index < 0)
            return nullptr;

        result = actor = actor->child(index).getPointer();
    }

    if (i < pathNode->childCount() || result->getType() != AST_OUTPUT)
        return nullptr;
    else
        return result;
}


/// <summary>
/// Removes 'typedef' intermediate nodes for named tuple definitions.
/// </summary>
Ref<AstNode> tupleRemoveTypedef(Ref<AstNode> node, SemAnalysisState& state)
{
    auto child = node->child(0);

    if (child->getType() != AST_TUPLE_DEF)
        return node;		//Nothing to do.
    else
    {
        child->setName(node->getName());
        return child;
    }
}

/// <summary>
/// Adds a 'tuple adapter node' for tuple assignments between compatible, 
/// but not the same type tuples
/// </summary>
Ref<AstNode> addTupleAdapter(Ref<AstNode> node, SemAnalysisState& state)
{
    auto lNode = node->child(0);
    auto lType = lNode->getDataType();
    auto rNode = node->child(1);

    node->setChild(1, makeTupleAdapter(rNode, lType));

    return node;
}

/// <summary>
/// Creates a tuple adapter node if necessary.
/// </summary>
/// <param name="rNode">Right hand node (the source node in an assignment)</param>
/// <param name="lType">Data type to which it shall be converted.</param>
/// <returns>An AST tuple adapter node or the right hand node if no adaptation is necessary.</returns>
Ref<AstNode> makeTupleAdapter(Ref<AstNode> rNode, AstNode* lType)
{
    auto rType = rNode->getDataType();

    if (!astIsTupleType(lType) && astIsTupleType(rType))
        return rNode;

    if (lType == rType)
        return rNode;

    auto adapterNode = astCreateTupleAdapter(rNode);
    adapterNode->setDataType(lType);

    return adapterNode;
}



/// <summary>
/// Adds a 'tuple adapter node' for tuple returned values which are compatible, 
/// but not the same, as the function return type.
/// </summary>
Ref<AstNode> addReturnTupleAdapter(Ref<AstNode> node, SemAnalysisState& state)
{
    if (!node->childExists(0))
        return node;

    auto functionNode = state.findParent([](auto node) {
        return node->getType() == AST_FUNCTION;
    });

    assert(functionNode.notNull());

    auto returnType = astGetReturnType(functionNode.getPointer());
    auto child = node->child(0);

    node->setChild(0, makeTupleAdapter(child, returnType));
    node->setDataType(returnType);

    return node;
}


/// <summary>Utility function to assign void type to a node.</summary>
/// <returns>Always 'ok'</returns>
CompileError setVoidType(Ref<AstNode> node, SemAnalysisState& state)
{
    node->setDataType(astGetVoid());
    return CompileError::ok();
}

/// <summary>
/// Checks and assignment operation.
/// This is not just used for assignment operations. It is used at any point in which 
/// data is copied or transferred (function / message calls, tuple creation, 
/// return statements, etc)
/// </summary>
/// <param name="lType">Left side (destination) data type.</param>
/// <param name="rExpr">Right side expression</param>
/// <returns>An operation result which may contain a compile error or the node
/// which should replace the right hand expression (it may be the same node)</returns>
SemanticResult assignCheck(AstNode* lType, Ref<AstNode> rExpr)
{
    switch (lType->getType())
    {
    case AST_FUNCTION_TYPE:
        return assignFunctionCheck(lType, rExpr);

    case AST_MESSAGE_TYPE:
        return assignMessageCheck(lType, rExpr);

    case AST_TUPLE_DEF:
    //case AST_TUPLE:
        return assignTupleCheck(lType, rExpr);

    default:
        return assignScalarCheck(lType, rExpr);
    }
}

/// <summary>
/// Checks an assignment operation.
/// This version modifies the node which contains the assignment operation if a 
/// transform is required.
/// </summary>
/// <param name="lType">Left side (destination) data type.</param>
/// <param name="parent">Operation (parent) node</param>
/// <param name="rightIndex">Index of the right-hand expression inside 'parent' node.</param>
/// <returns></returns>
CompileError assignCheck(AstNode* lType, Ref<AstNode> parent, int rightIndex)
{
    auto r = assignCheck(lType, parent->child(rightIndex));

    if (r.ok())
    {
        assert(r.result.notNull());
        parent->setChild(rightIndex, r.result);
        return CompileError::ok();
    }
    else
        return r.errors[0];
}

/// <summary>
/// Checks and assignment to a 'C' pointer
/// </summary>
SemanticResult assignCpointerCheck(AstNode* lType, Ref<AstNode> rExpr)
{
    auto rType = rExpr->getDataType();

    if (astIsCpointer(rType))
        return SemanticResult(rExpr);
    else if (astIsTupleType(rType))
    {
        auto adapter = astCreateGetAddress(rExpr->position(), rExpr);
        adapter->setDataType(astGetCPointer());
        return SemanticResult(adapter);
    }
    else
        return incompatibleTypesError(lType, rExpr);
}


/// <summary>
/// Checks and assignment to a function variable.
/// </summary>
SemanticResult assignFunctionCheck(AstNode* lType, Ref<AstNode> rExpr)
{
    auto rType = rExpr->getDataType();
    auto tA = lType->getType();
    auto tB = rType->getType();

    if (tB == AST_FUNCTION_TYPE || tB == AST_FUNCTION)
    {
        if (areTypesCompatible(astGetReturnType(lType), astGetReturnType(rType)))
        {
            if (areTuplesCompatible(astGetParameters(lType), astGetParameters(rType)))
                return SemanticResult(rExpr);
        }
    }

    return incompatibleTypesError(lType, rExpr);
}

/// <summary>
/// Checks and assignment to a message reference variable.
/// </summary>
SemanticResult assignMessageCheck(AstNode* lType, Ref<AstNode> rExpr)
{
    auto rType = rExpr->getDataType();
    auto tA = lType->getType();
    auto tB = rType->getType();

    if (tB == AST_MESSAGE_TYPE || tB == AST_INPUT)
        if (areTuplesCompatible(astGetParameters(lType), astGetParameters(rType)))
            return SemanticResult(rExpr);

    return incompatibleTypesError(lType, rExpr);
}

/// <summary>
/// Checks an assignment to an scalar value.
/// </summary>
SemanticResult assignScalarCheck(AstNode* lType, Ref<AstNode> rExpr)
{
    //TODO: implement conversion between numeric types... Oh, wait... We have only a
    //numeric type at this moment :-P
    auto rType = rExpr->getDataType();

    if (astIsCpointer(lType))
        return assignCpointerCheck(lType, rExpr);
    else if (lType->getType() == rType->getType())
    {
        if (lType->getName() == rType->getName())
            return SemanticResult(rExpr);
    }

    return incompatibleTypesError(lType, rExpr);
}

/// <summary>
/// Checks the assignment of an scalar to an one element tuple.
/// </summary>
SemanticResult assignScalarToTupleCheck(AstNode* lType, Ref<AstNode> rExpr)
{
    auto rType = rExpr->getDataType();

    if (lType->childCount() == 1)
    {
        //Try to assign an scalar to a tuple.
        auto r = assignCheck(lType->child(0)->getDataType(), rExpr);

        if (r.ok())
        {
            auto adapter = astCreateTuple(rExpr->position());
            adapter->addChild(rExpr);
            adapter->setDataType(adapter.getPointer());
            return SemanticResult(adapter);
        }
    }
    return incompatibleTypesError(lType, rExpr);
}


/// <summary>
/// Checks an assignment to a tuple.
/// </summary>
SemanticResult assignTupleCheck(AstNode* lType, Ref<AstNode> rExpr)
{
    //TODO: This function can produce more than a single error, but only the first one is reported.
    auto rType = rExpr->getDataType();

    if (!astIsTupleType(rType))
        return assignScalarToTupleCheck(lType, rExpr);
    else if (rExpr->getType() == AST_TUPLE)
    {
        if (lType->childCount() >= rType->childCount())
        {
            auto                    newTuple = astCreateTuple(rExpr->position());
            vector<CompileError>    errors;
            size_t                  i;

            for (i = 0; i < rType->childCount(); ++i)
            {
                auto r = assignCheck(lType->child(i)->getDataType(), rExpr->child(i));

                if (r.ok())
                    newTuple->addChild(r.result);
                else
                    r.appendErrorsTo(errors);
            }

            //TODO: Implement tuple default values.
            if (i < lType->childCount())
                return incompatibleTypesError(lType, rExpr);

            //Same type, of course.
            newTuple->setDataType(rExpr->getDataType());

            if (!errors.empty())
                return SemanticResult(errors);
            else
                return SemanticResult(newTuple);
        }
    }
    else
    {
        if (areTuplesCompatible(lType, rType))
            return SemanticResult(rExpr);
    }

    return incompatibleTypesError(lType, rExpr);
}

/// <summary>
/// Generates an 'Incompatible types' error.
/// </summary>
/// <param name="lType">Left side type.</param>
/// <param name="rExpr">Right side expression.</param>
/// <returns></returns>
SemanticResult incompatibleTypesError(AstNode* lType, Ref<AstNode> rExpr)
{
    return semError(rExpr,
        ETYPE_INCOMPATIBLE_TYPES_2,
        astTypeToString(rExpr->getDataType()).c_str(),
        astTypeToString(lType).c_str());
}



/// <summary>Gets the common type for two types.</summary>
/// <returns>Common type or 'null' if no common type can be found.</returns>
AstNode* getCommonType(AstNode* typeA, AstNode* typeB, SemAnalysisState& state)
{
    //TODO: This is a very basic implementation. Improve it.
    bool r = false;

    if (astIsTupleType(typeA))
        r = areTuplesCompatible(typeA, typeB);
    else
        r = areTypesCompatible(typeA, typeB);

    return r ? typeA : nullptr;
}

/// <summary>
/// Checks if two types are assignable without performing transformations. 
/// </summary>
/// <param name="typeA"></param>
/// <param name="typeB"></param>
/// <returns></returns>
bool areTypesCompatible(AstNode* typeA, AstNode* typeB)
{
    auto r = assignCheck(typeA, typeB);

    if (!r.ok())
        return false;
    else
        return  typeB == r.result.getPointer();
}

/// <summary>
/// Checks if two tuples are assignable without performing transforms.
/// </summary>
/// <param name="typeA"></param>
/// <param name="typeB"></param>
/// <returns></returns>
bool areTuplesCompatible(AstNode* typeA, AstNode* typeB)
{
    assert(astIsTupleType(typeA) && astIsTupleType(typeB));

    if (typeA->childCount() != typeB->childCount())
        return false;
    else
    {
        const int count = typeA->childCount();

        for (int i = 0; i < count; ++i)
        {
            auto childAType = typeA->child(i)->getDataType();
            auto childBType = typeB->child(i)->getDataType();
            bool r;

            if (astIsTupleType(childAType))
                r = areTuplesCompatible(childAType, childBType);
            else
                r = areTypesCompatible(childAType, childBType);

            if (!r)
                return false;
        }

        return true;
    }//else
}

/// <summary>
/// Checks whether an AST node represents a type or not.
/// </summary>
/// <param name="node"></param>
/// <returns></returns>
bool isType(Ref<AstNode> node)
{
    AstNodeTypes type = node->getType();

    return type == AST_DEFAULT_TYPE
        || type == AST_FUNCTION
        || type == AST_TUPLE_DEF
        || type == AST_ACTOR;
}
