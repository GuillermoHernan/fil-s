#include "pch.h"
#include "typeCheckPass.h"
#include "semanticAnalysis_internal.h"
#include "symbolScope.h"
#include "passOperations.h"

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

		functions.add(AST_BLOCK, blockTypeCheck);
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
	}

	return semInOrderWalk(functions, state, node);
}

/// <summary>
/// Checks that the referenced type exists
/// </summary>
/// <param name="node"></param>
/// <param name="state"></param>
/// <returns></returns>
CompileError typeExistsCheck(Ref<AstNode> node, SemAnalysisState& state)
{
	auto	scope = node->getScope();
	string	name = node->getName();

	auto typeNode = scope->get(name, true);

	if (typeNode.isNull())
		return semError(node, ETYPE_NON_EXISTENT_SYMBOL_1, name.c_str());
	
	if (!isType(typeNode))
		return semError(node, ETYPE_NOT_A_TYPE_1, name.c_str());

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
/// Type check on a tuple creation node. Builds an unnamed tuple definition as type.
/// </summary>
CompileError tupleTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
	auto	tupleDataType = refFromNew(new AstBranchNode(AST_TUPLE_DEF, node->position()));

	for (auto child : node->children())
		tupleDataType->addChild(child->getDataType());

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
		return semError(node->child(0), ETYPE_WRONG_IF_CONDITION_TYPE_1, getTypeText(conditionType).c_str());

	auto thenType = node->child(1)->getDataType();

	if (!node->childExists(2))
		node->setDataType(thenType);
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
/// Performs type checking on a function definition.
/// </summary>
CompileError functionDefTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
	auto bodyType = node->child(2)->getDataType();

	//functions define ots own type.
	node->setDataType(node);

	//Infer return type if necessary.
	if (!node->childExists(1))
	{
		node->setChild(1, bodyType);
		return CompileError::ok();
	}
	else
	{
		auto declaredType = node->child(1)->getDataType();

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
	auto fnType = node->child(0)->getDataType();
	auto paramsType = node->child(1)->getDataType();

	assert(fnType->getType() == AST_FUNCTION);

	node->setDataType(fnType->child(1));
	return areTypesCompatible(fnType->child(0), paramsType, node);
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
		node->setDataType(referenced);
		return CompileError::ok();
	}
}

/// <summary>Type checking of member access operations.</summary>
CompileError memberAccessTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
	auto leftType = node->child(0)->getDataType();
	string name = node->child(1)->getName();

	auto child = leftType->findChildByName(name);

	if (child.isNull())
		return semError(node->child(1), ETYPE_MEMBER_NOT_FOUND_2, name.c_str(), getTypeText(leftType).c_str());
	else
	{
		node->setDataType(child);
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
	else if (op == "<" || op == ">" || op == "==" || op == "!=" || op == ">=" || op == "<=")
		return comparisionOperatorTypeCheck(node, state);
	else
		return logicalOperatorTypeCheck(node, state);
}

/// <summary>Type check for prefix operators</summary>
CompileError prefixOpTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
	string op = node.staticCast<AstOperator>()->operation;
	auto childType = node->child(0)->getDataType();

	node->setDataType(childType);

	if (op == "!" && !isBoolType(childType))
		return semError(node->child(0), ETYPE_WRONG_TYPE_2, getTypeText(childType).c_str(), "bool");
	else if (!isIntType(childType))
		return semError(node->child(0), ETYPE_WRONG_TYPE_2, getTypeText(childType).c_str(), "int");
	else
		return CompileError::ok();
}

/// <summary>Type check for postfix operators</summary>
CompileError postfixOpTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
	auto childType = node->child(0)->getDataType();

	node->setDataType(childType);

	if (!isIntType(childType))
		return semError(node->child(0), ETYPE_WRONG_TYPE_2, getTypeText(childType).c_str(), "int");
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
		return semError(node->child(0), ETYPE_WRONG_TYPE_2, getTypeText(lexpr->getDataType()).c_str(), "int");
	else if (!isIntType(rexpr->getDataType()))
		return semError(node->child(0), ETYPE_WRONG_TYPE_2, getTypeText(rexpr->getDataType()).c_str(), "int");
	else
		return CompileError::ok();
}

/// <summary>Type check for bitrwise operators</summary>
CompileError bitwiseOperatorTypeCheck(Ref<AstNode> node, SemAnalysisState& state)
{
	//It is the same, by the moment.
	return mathOperatorTypeCheck(node, state);
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
