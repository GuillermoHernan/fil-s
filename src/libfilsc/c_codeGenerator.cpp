/// <summary>
/// Generates 'C' language code from the checked program AST.
/// </summary>
/// <remarks>
/// It does not try to perform much optimization, as the 'C' compiler is suppossed 
/// to be good optimizing code.
/// </remarks>

#include "pch.h"
#include "c_codeGenerator_internal.h"
#include "compileError.h"
#include "SymbolScope.h"
#include "utils.h"
#include "codeGeneratorState.h"

using namespace std;

/// <summary>
/// 'C' code generation entry point. Generates 'C' source from the AST.
/// </summary>
/// <param name="node">AST root</param>
/// <returns>A string containing 'C' source code.</returns>
string generateCode(Ref<AstNode> node)
{
	return generateCode(node, [](auto node) {return false; });
}

/// <summary>
/// 'C' code generation entry point. Generates 'C' source from the AST.
/// It allows to specify the entry point.
/// </summary>
/// <param name="node">AST root</param>
/// <param name="entryPointFn">Function to look for the entry point. The entry point
/// has the same name in 'FIL-S' and 'C' sources.</param>
/// <returns></returns>
string generateCode(Ref<AstNode> node, std::function<bool(Ref<AstNode>)> entryPointFn)
{
	ostringstream		output;
	CodeGeneratorState	state(&output);

	auto &topLevelItems = node->children();
	auto it = find_if(topLevelItems.begin(), topLevelItems.end(), entryPointFn);

	if (it != topLevelItems.end())
		state.setCname(*it, (*it)->getName());

	codegen(node, state, VoidVariable());

	return output.str();
}

/// <summary>
/// Generates code for an AST node. Based on the node type, selects the appropriate 
/// code generation function.
/// </summary>
/// <remarks>
/// When adding new AST nodes, the table in this function shall be updated.
/// </remarks>
/// <param name="node"></param>
/// <param name="output"></param>
/// <param name="state"></param>
/// <param name="resultDest">Name of the 'C' variable where the result of the expression
/// should be stored.
/// </param>
void codegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest)
{
	static NodeCodegenFN types[AST_TYPES_COUNT] = { NULL, NULL };

	if (types[0] == NULL)
	{
		//Default everything to 'invalid node'. To gracefully handle the bugs caused
		//by not keeping this list updated ;-)
		fill_n(types, AST_TYPES_COUNT, invalidNodeCodegen);

		types[AST_SCRIPT]		= nodeListCodegen;
		types[AST_TYPEDEF]		= voidCodegen;
		types[AST_LIST]			= invalidNodeCodegen;
		types[AST_BLOCK]		= blockCodegen;
		types[AST_TUPLE]		= tupleCodegen;
		types[AST_DECLARATION]	= varCodegen;
		types[AST_TUPLE_DEF] = tupleDefCodegen;
		types[AST_TUPLE_ADAPTER] = tupleAdapterCodegen;
		types[AST_IF]			= ifCodegen;
		types[AST_FOR]			= invalidNodeCodegen;
		types[AST_FOR_EACH]		= invalidNodeCodegen;
		types[AST_RETURN]		= returnCodegen;
		types[AST_FUNCTION]		= functionCodegen;
		types[AST_ASSIGNMENT]	= assignmentCodegen;
		types[AST_FNCALL]		= callCodegen;
		types[AST_INTEGER]		= literalCodegen;
		types[AST_FLOAT]		= literalCodegen;
		types[AST_STRING]		= literalCodegen;
		types[AST_BOOL]			= literalCodegen;
		types[AST_IDENTIFIER]	= varAccessCodegen;
		types[AST_ARRAY]		= invalidNodeCodegen;
		types[AST_ARRAY_ACCESS]	= invalidNodeCodegen;
		types[AST_MEMBER_ACCESS]= memberAccessCodegen;
		types[AST_BINARYOP]		= binaryOpCodegen;
		types[AST_PREFIXOP]		= prefixOpCodegen;
		types[AST_POSTFIXOP]	= postfixOpCodegen;
		types[AST_ACTOR]		= invalidNodeCodegen;
		types[AST_CONNECT]		= invalidNodeCodegen;
		types[AST_DEFAULT_TYPE] = invalidNodeCodegen;
		types[AST_TYPE_NAME]	= voidCodegen;
	}

	if (node.notNull())
		types[node->getType()](node, state, resultDest);
}

/// <summary>
/// Code generation function for nodes which do not require code generation
/// </summary>
void voidCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest)
{
}

/// <summary>
/// Handles the case of node types which are not suppossed to reach code generation phase.
/// </summary>
void invalidNodeCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest)
{
	errorAt(node->position(),
		ETYPE_INVALID_CODEGEN_NODE_1,
		astTypeToString(node->getType()).c_str());
}

/// <summary>
/// Calls code generation for all children of the node.
/// </summary>
void nodeListCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest)
{
	assert(resultDest.isVoid());

	for (auto child : node->children())
		codegen(child, state, VoidVariable());
}

/// <summary>
/// Generates code for a function definition node.
/// </summary>
void functionCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest)
{
	//Necessary because functions usually define their own temporaries.
	state.enterBlock();

	//Generate code for the parameters tuple.
	auto params = node->child(0);
	auto retTuple = node->child(1);
	auto fnCode = node->child(2);
	auto returnType = node->getDataType().staticCast<FunctionType>()->getReturnType();

	if (params->childCount() > 0)
	{
		state.output() << "//Parameters for '" << node->getName() << "' function\n";
		codegen(params, state, VoidVariable());
	}

	if (returnType->type() == DT_TUPLE)
	{
		state.output() << "//Return value for '" << node->getName() << "' function\n";

		//TODO: This may not work for inferred return types.
		codegen(retTuple, state, VoidVariable());
	}

	state.output() << "//Code for '" << node->getName() << "' function\n";
	state.output() << genFunctionHeader(node, state);
	state.output() << "{\n";

	if(returnType->type() == DT_VOID)
		codegen(fnCode, state, VoidVariable());
	else
	{
		TempVariable	tmpReturn(returnType, state, false);
		codegen(fnCode, state, tmpReturn);

		state.output() << "return " << tmpReturn.cname() << ";\n";
	}

	state.output() << "}\n\n";

	state.exitBlock();
}

/// <summary>
/// Generates the 'C' header of a function.
/// </summary>
string genFunctionHeader(Ref<AstNode> node, CodeGeneratorState& state)
{
	auto	params = node->child(0);
	auto	type = node->getDataType().staticCast<FunctionType>();
	auto	retType = type->getReturnType();
	string	result;

	result.reserve(128);

	result = "static ";

	//Return type.
	if (retType->type() == DT_VOID)
		result += "void ";
	else 
		result += state.cname(retType) + " ";

	//Function name
	result += state.cname(node);

	//Parameters.
	if (params->childCount() == 0)
		result += "()";
	else
		result += "(" + state.cname(params) + "* _gen_params)";

	return result;
}

/// <summary>
/// Generates code for a block of expressions
/// </summary>
void blockCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest)
{
	if (node->childCount() == 0)
		return;

	const auto &	children = node->children();
	auto			lastChild = children.back();

	state.enterBlock();

	state.output() << "{\n";

	for (size_t i = 0; i < children.size() - 1; ++i)
		codegen(children[i], state, VoidVariable());

	codegen(lastChild, state, resultDest);

	state.output() << "}\n";

	state.exitBlock();
}

/// <summary>
/// Generates code for a tuple creation expression.
/// </summary>
void tupleCodegen(
	Ref<AstNode> node, 
	CodeGeneratorState& state, 
	const IVariableInfo& resultDest)
{
	//TODO: Review this check. Is really right not to evaluate the sub-expressions?
	if (resultDest.isVoid())
		return;

	auto&	expressions = node->children();
	auto	tupleType = resultDest.dataType().staticCast<TupleType>();

	for (size_t i = 0; i < expressions.size(); ++i)
	{
		auto		childNode = tupleType->getMemberNode((unsigned)i);
		TupleField	field(resultDest, i, state);

		codegen(expressions[i], state, field);
	}
}

/// <summary>
/// Generates code for a variable declaration.
/// </summary>
void varCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest)
{
	auto	typeNode = node->getDataType();
	
	if (typeNode->type() == DT_TUPLE && state.hasName(typeNode) == false)
		tupleDefCodegen(typeNode, state);

	state.output() << state.cname(typeNode) << " ";
	state.output() << state.cname(node) << ";\n";

	if (node->childExists(1))
		codegen(node->child(1), state, NamedVariable(node, state));
}

/// <summary>
/// Generates code for a tuple definition.
/// </summary>
void tupleDefCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest)
{
	string name = state.cname(node);

	state.output() << "typedef struct " << "{\n";

	nodeListCodegen(node, state, VoidVariable());

	state.output() << "}" << name << ";\n\n";
}
void tupleDefCodegen(Ref<BaseType> type, CodeGeneratorState& state)
{
	assert(type->type() == DT_TUPLE);
	string name = state.cname(type);

	state.output() << "typedef struct " << "{\n";

	auto tType = type.staticCast<TupleType>();
	const int count = tType->memberCount();
	for (int i = 0; i < count; ++i)
		codegen(tType->getMemberNode(i), state, VoidVariable());

	state.output() << "}" << name << ";\n\n";
}

/// <summary>
/// Generates code for a tuple adapter node.
/// </summary>
void tupleAdapterCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest)
{
	assert(!resultDest.isVoid());

	//TODO: This is not going to work when default tuple values are implemented.
	//(or other more complex type-adapting features)
	TempVariable	rTemp(node->child(0), state, false);
	string			lName = resultDest.cname();

	codegen(node->child(0), state, rTemp);
	state.output() << "memcpy (&" << lName << ", &" << rTemp.cname() 
		<< ", sizeof(" << resultDest.cname() << "));\n";

}


/// <summary>
/// Generates code for an 'if' flow control expression.
/// </summary>
void ifCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest)
{
	auto	condition = node->child(0);
	auto	thenExpr = node->child(1);
	auto	elseExpr = node->child(2);

	//Condition
	TempVariable conditionTempVar(condition, state, false);

	codegen(condition, state, conditionTempVar);

	state.output() << "if(" << conditionTempVar.cname() << "){\n";

	//Then
	codegen(thenExpr, state, resultDest);
	state.output() << "}\n";

	//Else (optional)
	if (elseExpr.notNull())
	{
		state.output() << "else{\n";
		codegen(elseExpr, state, resultDest);
		state.output() << "}\n";
	}
}

/// <summary>
/// Generates code for a return statement
/// </summary>
void returnCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest)
{
	if (!node->childExists(0))
		state.output() << "return;\n";
	else
	{
		auto			expression = node->child(0);
		TempVariable	tempVar(node, state, false);

		codegen(expression, state, tempVar);
		state.output() << "return " << tempVar.cname() << ";\n";
	}
}

/// <summary>
/// Generates code for an assignment expression.
/// </summary>
void assignmentCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest)
{
	auto	assignment = node.staticCast<AstOperator>();
	auto	lexpr = assignment->child(0);
	auto	rexpr = assignment->child(1);

	assert(assignment->operation == "=");

	TempVariable	lRef(lexpr, state, true);
	TempVariable	rResult(rexpr, state, false);
	
	codegen(lexpr, state, lRef);
	codegen(rexpr, state, rResult);

	state.output() << "*" << lRef << " = " << rResult << ";\n";
	if (!resultDest.isVoid())
		state.output() << resultDest << " = " << rResult << ";\n";

	//if (lexpr->getType() == AST_IDENTIFIER)
	//{
	//	auto			referenced = node->getScope()->get(lexpr->getName());
	//	NamedVariable	destination(referenced, state);

	//	codegen(rexpr, state, destination);
	//	if (!resultDest.isVoid())
	//		state.output() << resultDest.cname() << " = " << destination.cname() << ";\n";
	//}
	//else
	//{
	//	//By the moment, only 'member access' nodes are sopported.
	//	assert(lexpr->getType() == AST_MEMBER_ACCESS);
	//	TempVariable	tempResult(node, state);

	//	codegen(rexpr, state, tempResult);
	//	memberAccessCodegen(lexpr, state, tempResult, true);

	//	if (!resultDest.isVoid())
	//		state.output() << resultDest.cname() << " = " << tempResult.cname() << ";\n";
	//}
}

/// <summary>
/// Generates code for a function call expression.
/// </summary>
void callCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest)
{
	auto fnExpr = node->child(0);
	auto paramsExpr = node->child(1);

	//By the moment, only direct function invocation is sopported
	assert(fnExpr->getType() == AST_IDENTIFIER);

	auto	fnNode = node->getScope()->get(fnExpr->getName(), true);
	string	fnCName = state.cname(fnNode);

	if (paramsExpr->childCount() == 0)
	{
		assert(resultDest.isVoid());

		state.output() << fnCName << "();\n";
	}
	else
	{
		auto			fnType = fnNode->getDataType().staticCast<FunctionType>();
		auto			paramsType = fnType->getParameters();
		TempVariable	tmpParams(paramsType.staticCast<BaseType>(), state, false);

		codegen(paramsExpr, state, tmpParams);

		if (!resultDest.isVoid())
			state.output() << resultDest.cname() << " = ";

		state.output() << fnCName << "(&" << tmpParams.cname() << ");\n";
	}
}

/// <summary>
/// Generates code for a literal node
/// </summary>
void literalCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest)
{
	if (resultDest.isVoid())
		return;

	switch (node->getType())
	{
	case AST_INTEGER:
	case AST_FLOAT:
		state.output() << resultDest.cname() << " = " << node->getValue() << ";\n";
		break;

	case AST_STRING:
		state.output() << resultDest.cname() << " = " << escapeString (node->getValue(), true) << ";\n";
		break;

	default:
		assert(!"Unexpected literal type");
	}
}

/// <summary>
/// Generates code to read a variable.
/// </summary>
void varAccessCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest)
{
	if (resultDest.isVoid())
		return;

	string accessExpression = varAccessExpression(node, state);
	if (resultDest.isReference)
		state.output() << resultDest.cname() << " = &" << accessExpression << ";\n";
	else
		state.output() << resultDest.cname() << " = " << accessExpression << ";\n";
}

/// <summary>
/// Generates code for an access to member expression.
/// </summary>
void memberAccessCodegen(
	Ref<AstNode> node, 
	CodeGeneratorState& state, 
	const IVariableInfo& variable)
{
	vector <string>	nameStack;
	auto curNode = node;
	AstNodeTypes	type;

	do {
		type = curNode->getType();

		if (type == AST_IDENTIFIER)
			nameStack.push_back(varAccessExpression(curNode, state));
		else if (type == AST_MEMBER_ACCESS)
		{
			auto tuple = curNode->child(0);
			auto tType = tuple->getDataType().staticCast<TupleType>();
			string fieldName = curNode->child(1)->getName();
			int index = tType->findMemberByName(fieldName);

			if (index < 0)
				errorAt(node->position(), ETYPE_MEMBER_NOT_FOUND_2, fieldName.c_str(), tType->toString().c_str());

			nameStack.push_back(state.cname(tType->getMemberNode(index)));
			curNode = tuple;
		}
		else
		{
			TempVariable	temp(curNode, state, false);

			codegen(curNode, state, temp);
			nameStack.push_back(temp.cname());
		}
	} while (type == AST_MEMBER_ACCESS);

	assert(nameStack.size() >= 2);

	if (!variable.isVoid())
	{
		std::reverse(nameStack.begin(), nameStack.end());

		state.output() << variable << " = ";
		if (variable.isReference)
			state.output() << "&";
		state.output() << join(nameStack, ".") << ";\n";
	}
}

/// <summary>
/// Generates code for a binary operation
/// </summary>
void binaryOpCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest)
{
	if (resultDest.isVoid())
		return;

	auto leftExpr = node->child(0);
	auto rightExpr = node->child(1);
	auto operation = node.staticCast<AstOperator>()->operation;

	TempVariable	leftTmp(leftExpr, state, false);
	TempVariable	rightTmp(rightExpr, state, false);

	codegen(leftExpr, state, leftTmp);
	codegen(rightExpr, state, rightTmp);

	state.output() << resultDest << " = ";
	state.output() << leftTmp.cname() << operation << rightTmp.cname() << ";\n";
}


/// <summary>
/// Generates code for a prefix operator
/// </summary>
void prefixOpCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest)
{
	auto child = node->child(0);
	auto operation = node.staticCast<AstOperator>()->operation;
	bool needsRef = (operation == "++" || operation == "--");

	TempVariable	temp(child, state, needsRef);

	codegen(child, state, temp);

	if (!resultDest.isVoid())
		state.output() << resultDest << " = ";
	
	state.output() << operation;
	if (needsRef)
		state.output() << "*";
	state.output() << temp << ";\n";
}


/// <summary>
/// Generates code for a postfix operator
/// </summary>
void postfixOpCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest)
{
	auto child = node->child(0);
	auto operation = node.staticCast<AstOperator>()->operation;

	TempVariable	temp(child, state, true);

	codegen(child, state, temp);

	state.output() << operation << "(*" << temp << ");\n";

	if (!resultDest.isVoid())
		state.output() << resultDest << " = *" << temp.cname() << ";\n";
}

/// <summary>
/// Generates the expression need to access a variable. 
/// It returns it, it does not write it on the output
/// </summary>
/// <param name="node"></param>
/// <param name="state"></param>
/// <returns></returns>
std::string varAccessExpression(Ref<AstNode> node, CodeGeneratorState& state)
{
	string namePrefix = "";

	auto referenced = node->getScope()->get(node->getName());

	if (referenced->hasFlag(ASTF_FUNCTION_PARAMETER))
		namePrefix = "_gen_params->";

	return namePrefix + state.cname(referenced);
}
