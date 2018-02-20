#include "pch.h"
#include "typeCheckPass.h"
#include "semanticAnalysis_internal.h"
#include "symbolScope.h"
#include "passOperations.h"
#include "dataTypes.h"
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
		functions.add(AST_ASSIGNMENT, assignmentTypeCheck);
		functions.add(AST_FNCALL, callTypeCheck);
		functions.add(AST_INTEGER, literalTypeAssign);
		functions.add(AST_FLOAT, literalTypeAssign);
		functions.add(AST_STRING, literalTypeAssign);
		functions.add(AST_BOOL, literalTypeAssign);
		functions.add(AST_IDENTIFIER, varReadTypeCheck);
		functions.add(AST_MEMBER_ACCESS, memberAccessTypeCheck);
		functions.add(AST_BINARYOP, binaryOpTypeCheck);
		functions.add(AST_PREFIXOP, prefixOpTypeCheck);
		functions.add(AST_POSTFIXOP, postfixOpTypeCheck);
		functions.add(AST_DEFAULT_TYPE, defaultTypeAssign);

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
	auto referenced = node->getScope()->get(node->getName(), false);
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
	auto	scope = node->getScope();
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
	auto	scope = node->getScope();
	auto	tupleType = TupleType::create(node->getName());

	for (auto child : node->children())
		tupleType->addMember(child);

	node->setDataType(tupleType);

	return CompileError::ok();
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
	auto	tupleDataType = TupleType::create();

	for (auto child : node->children())
	{
		auto newDeclNode = astCreateDeclaration(child->position(), "", Ref<AstNode>(), Ref<AstNode>());
		newDeclNode->setDataType(child->getDataType());
		tupleDataType->addMember(newDeclNode);
	}

	node->setDataType(tupleDataType);

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
		node->setDataType(declaredType);

		if (!node->childExists(1))
			return CompileError::ok();	//No initialization
		else
			return areTypesCompatible(declaredType, node->child(1)->getDataType(), node);
	}
	else
	{
		if (!node->childExists(1))
			return semError( node, ETYPE_DECLARATION_WITHOUT_TYPE);
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

	if (!isBoolType(conditionType))
		return semError(node->child(0), ETYPE_WRONG_IF_CONDITION_TYPE_1, conditionType->toString().c_str());

	auto thenType = node->child(1)->getDataType();

	if (!node->childExists(2))
		setVoidType(node, state);
	else
	{
		auto elseType = node->child(2)->getDataType();
		auto common = getCommonType(thenType, elseType, state);

		if (common.notNull())
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

	auto returnType = functionNode->getDataType().staticCast<FunctionType>()->getReturnType();

	if (!areTypesCompatible(returnType, node->getDataType()))
	{
		return semError(node, ETYPE_INCOMPATIBLE_RETURN_TYPE_2,
			node->getDataType()->toString().c_str(),
			returnType->toString().c_str());
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
	auto bodyType = body->getDataType();

	//Create function own data type.
	auto fnType = FunctionType::create(node);
	node->setDataType(fnType);

	//Infer return type if necessary.
	if (!node->childExists(1))
	{
		if (bodyType->type() == DT_TUPLE)
		{
			auto tupleType = bodyType.staticCast<TupleType>();
			auto tupleDefNode = buildTupleDefFromTupleType(tupleType, body->position());
			node->setChild(1, tupleDefNode);
		}
		fnType->setReturnType(bodyType);
		return CompileError::ok();
	}
	else
	{
		auto declaredType = fnType->getReturnType();

		//If void is declared explicitly, then any type is valid for the body.
		if (isVoidType(declaredType))
			return CompileError::ok();
		else
			return areTypesCompatible(declaredType, bodyType, node);
	}
}

/// <summary>Type checking for assignment operations.</summary>
CompileError assignmentTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
	auto leftType = node->child(0)->getDataType();
	auto rightType = node->child(1)->getDataType();

	node->setDataType(leftType);
	return areTypesCompatible(leftType, rightType, node);
}

/// <summary>Performs type checking for function calls.</summary>
CompileError callTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
	auto type = node->child(0)->getDataType();
	auto paramsType = node->child(1)->getDataType();

	assert(type->type() == DT_FUNCTION);
	auto fnType = type.staticCast<FunctionType>();

	node->setDataType(fnType->getReturnType());
	return areTypesCompatible(fnType->getParameters(), paramsType, node);
}

/// <summary>Type check for variable / symbol reading</summary>
/// <param name="node"></param>
/// <param name="state"></param>
/// <returns></returns>
CompileError varReadTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
	auto scope = node->getScope();

	auto referenced = scope->get(node->getName(), true);

	if (referenced.isNull())
		return semError(node, ETYPE_NON_EXISTENT_SYMBOL_1, node->getName().c_str());
	else
	{
		//TODO: There is a problem. the referenced node may not have an assigned type
		//at this moment. May this be the main problem for type inference?
		//Without type inference, we may just assign the types for declarations in a previous pass,
		//and perform expression type checking in a later pass.
		//Perhaps, a possible solution may be walking the tree on a different way...
		if (referenced->getDataType().isNull())
			return semError(node, ETYPE_NOT_IMPLEMENTED_1, "Resolving data types for later defined symbols");

		node->setDataType(referenced->getDataType());
		return CompileError::ok();
	}
}

/// <summary>Type checking of member access operations.</summary>
CompileError memberAccessTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
	auto	leftType = node->child(0)->getDataType();
	assert(leftType->type() == DT_TUPLE);
	
	auto	tupleType = leftType.staticCast<TupleType>();
	string	name = node->child(1)->getName();

	int index = tupleType->findMemberByName(name);

	if (index < 0)
		return semError(node->child(1), ETYPE_MEMBER_NOT_FOUND_2, name.c_str(), leftType->toString().c_str());
	else
	{
		node->setDataType(tupleType->getMemberType(index));
		return CompileError::ok();
	}
}

/// <summary>Type check for binary operators</summary>
CompileError binaryOpTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
	string op = node.staticCast<AstOperator>()->operation;

	if (op == "+" || op == "-" ||op == "*" || op == "/" || op == "%")
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
	string op = node.staticCast<AstOperator>()->operation;
	auto childType = node->child(0)->getDataType();

	node->setDataType(childType);

	if (op == "!")
	{
		if (!isBoolType(childType))
			return semError(node->child(0), ETYPE_WRONG_TYPE_2, childType->toString().c_str(), "bool");
	}
	else
	{
		if (!isIntType(childType))
			return semError(node->child(0), ETYPE_WRONG_TYPE_2, childType->toString().c_str(), "int");
	}

	return CompileError::ok();
}

/// <summary>Type check for postfix operators</summary>
CompileError postfixOpTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
	auto childType = node->child(0)->getDataType();

	node->setDataType(childType);

	if (!isIntType(childType))
		return semError(node->child(0), ETYPE_WRONG_TYPE_2, childType->toString().c_str(), "int");
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

	if (!isIntType(lexpr->getDataType()))
		return semError(node->child(0), ETYPE_WRONG_TYPE_2, lexpr->getDataType()->toString().c_str(), "int");
	else if (!isIntType(rexpr->getDataType()))
		return semError(node->child(0), ETYPE_WRONG_TYPE_2, rexpr->getDataType()->toString().c_str(), "int");
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

	node->setDataType(DefaultType::createBool());
	return result;
}

/// <summary>Type check for equality and inequality operators</summary>
CompileError equalityOperatorTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
	//TODO: Support other types different from integers and booleans.
	auto lexpr = node->child(0);
	auto rexpr = node->child(1);

	node->setDataType(DefaultType::createBool());

	if (isIntType(lexpr->getDataType()))
	{
		if (!isIntType(rexpr->getDataType()))
			return semError(rexpr, ETYPE_WRONG_TYPE_2, rexpr->getDataType()->toString().c_str(), "int");
	}
	else if (isBoolType(lexpr->getDataType()))
	{
		if (!isBoolType(rexpr->getDataType()))
			return semError(rexpr, ETYPE_WRONG_TYPE_2, rexpr->getDataType()->toString().c_str(), "bool");
	}
	else
		return semError(lexpr, ETYPE_WRONG_TYPE_2, rexpr->getDataType()->toString().c_str(), "int or bool");

	//If ir reach here, check is ok.
	return CompileError::ok();
}

/// <summary>Type check for logical operators</summary>
CompileError logicalOperatorTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
	auto lexpr = node->child(0);
	auto rexpr = node->child(1);

	node->setDataType(lexpr->getDataType());

	if (!isBoolType(lexpr->getDataType()))
		return semError(node->child(0), ETYPE_WRONG_TYPE_2, lexpr->getDataType()->toString().c_str(), "bool");
	else if (!isBoolType(rexpr->getDataType()))
		return semError(node->child(0), ETYPE_WRONG_TYPE_2, rexpr->getDataType()->toString().c_str(), "bool");
	else
		return CompileError::ok();
}

/// <summary>Assigns the type for a literal expression.</summary>
CompileError literalTypeAssign(Ref<AstNode> node, SemAnalysisState& state)
{
	switch (node->getType())
	{
	case AST_BOOL:
		node->setDataType(DefaultType::createBool());
		break;

	case AST_INTEGER:
		node->setDataType(DefaultType::createInt());
		break;

	case AST_STRING:
		return semError(node, ETYPE_NOT_IMPLEMENTED_1, "String literals");
		break;

	default:
		assert(!"Unexpected literal type");
	}

	return CompileError::ok();
}

/// <summary>Assigns the type for a default type node.</summary>
CompileError defaultTypeAssign(Ref<AstNode> node, SemAnalysisState& state)
{
	//TODO: Think in better alternatives to handle default types. The current one
	//is weird. A hack.
	string name = node->getName();

	if (name == "int")
		node->setDataType(DefaultType::createInt());
	else
	{
		assert(name == "bool");
		node->setDataType(DefaultType::createBool());
	}

	return CompileError::ok();
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

	if (lType->type() != DT_TUPLE)
		return node;

	auto rNode = node->child(1);
	auto rType = rNode->getDataType();

	if (lType == rType)
		return node;

	auto adapterNode = astCreateTupleAdapter(rNode);
	adapterNode->setDataType(lType);

	node->setChild(1, adapterNode);
	return node;
}


/// <summary>
/// Adds a 'tuple adapter node' for tuple returned values which are compatible, 
/// but not the same, as the function return type.
/// </summary>
Ref<AstNode> addReturnTupleAdapter(Ref<AstNode> node, SemAnalysisState& state)
{
	auto functionNode = state.findParent([](auto node) {
		return node->getType() == AST_FUNCTION;
	});

	assert(functionNode.notNull());

	auto returnType = functionNode->getDataType().staticCast<FunctionType>()->getReturnType();

	if (returnType->type() != DT_TUPLE || returnType == node->getDataType())
		return node;

	auto child = node->child(0);
	auto adapterNode = astCreateTupleAdapter(child);
	
	adapterNode->setDataType(returnType);
	node->setDataType(returnType);
	node->setChild(0, adapterNode);

	return node;
}


/// <summary>Utility function to assign void type to a node.</summary>
/// <returns>Always 'ok'</returns>
CompileError setVoidType(Ref<AstNode> node, SemAnalysisState& state)
{
	node->setDataType(DefaultType::createVoid());
	return CompileError::ok();
}

/// <summary>Checks if an object of 'typeB' can be assigned to a object of 'typeA'</summary>
/// <param name="typeA">Type of the object which is going to the receive the value.</param>
/// <param name="typeB">Type of the source object.</param>
/// <param name="opNode">Used to create the compile error (to give a location to it)</param>
/// <returns>A compile error or 'ok'</returns>
CompileError areTypesCompatible(Ref<BaseType> typeA, Ref<BaseType> typeB, Ref<AstNode> opNode)
{
	bool result = areTypesCompatible(typeA, typeB);

	if (result)
		return CompileError::ok();
	else
	{
		if (typeA->type() == DT_FUNCTION)
			return semError(opNode, ETYPE_NOT_IMPLEMENTED_1, "Function variables assigning");
		else
			return semError(opNode, ETYPE_INCOMPATIBLE_TYPES_2, typeB->toString().c_str(), typeA->toString().c_str());
	}
}

/// <summary>Checks if an object of 'typeB' can be assigned to a object of 'typeA'</summary>
/// <param name="typeA">Type of the object which is going to the receive the value.</param>
/// <param name="typeB">Type of the source object.</param>
/// <returns>true if types are compatible'</returns>
bool areTypesCompatible(Ref<BaseType> typeA, Ref<BaseType> typeB)
{
	if (typeA->type() != typeB->type())
		return false;
	else
	{
		switch (typeA->type())
		{
		case DT_TUPLE:
			return areTuplesCompatible(typeA.staticCast<TupleType>(), typeB.staticCast<TupleType>());
		case DT_FUNCTION:
			return false;
		default:
			return true;
		}
	}
}

/// <summary>Checks if a tuple of 'tupleB' can be assigned to a tuple of 'tupleA'</summary>
/// <param name="typeA">tuple which is going to the receive the value.</param>
/// <param name="typeB">Source tuple.</param>
/// <returns>true if the tuples are compatible</returns>
bool areTuplesCompatible(Ref<TupleType> tupleA, Ref<TupleType> tupleB)
{
	if (tupleA->memberCount() != tupleB->memberCount())
		return false;
	else
	{
		const int count = tupleA->memberCount();

		for (int i = 0; i < count; ++i)
		{
			if (!areTypesCompatible(tupleA->getMemberType(i), tupleB->getMemberType(i)))
				return false;
		}

		return true;
	}
}

/// <summary>Gets the common type for two types.</summary>
/// <returns>Common type or 'null' if no common type can be found.</returns>
Ref<BaseType> getCommonType(Ref<BaseType> typeA, Ref<BaseType> typeB, SemAnalysisState& state)
{
	//TODO: This is a very basic implementation. Improve it.

	if (typeA->type() != typeB->type())
		return Ref<BaseType>();
	else
	{
		switch (typeA->type())
		{
		case DT_FUNCTION:
			return Ref<BaseType>();

		case DT_TUPLE:
			if (areTuplesCompatible(typeA.staticCast<TupleType>(), typeB.staticCast<TupleType>()))
				return typeA;
			else
				return Ref<BaseType>();

		default:
			return typeA;
		}
	}
}

/// <summary>
/// Builds an AST Tuple definition node from a tuple data type.
/// It is used for type inference.
/// </summary>
/// <param name="tuple"></param>
/// <returns></returns>
Ref<AstNode> buildTupleDefFromTupleType(Ref<TupleType> tuple, const ScriptPosition& pos)
{
	auto result = astCreateTupleDef(pos, "");

	int count = tuple->memberCount();
	for (int i = 0; i < count; ++i)
		result->addChild(tuple->getMemberNode(i));

	result->setDataType(tuple);

	return result;
}


/// <summary>Checks if a type is boolean</summary>
bool isBoolType(Ref<BaseType> type)
{
	return type->type() == DT_BOOL;
}

/// <summary>Checks if a type is integer</summary>
bool isIntType(Ref<BaseType> type)
{
	return type->type() == DT_INT;
}

/// <summary>Checks if a type is boolean</summary>
bool isVoidType(Ref<BaseType> type)
{
	return type->type() == DT_VOID;
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
