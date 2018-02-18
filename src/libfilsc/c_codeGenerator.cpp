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
	CodeGeneratorState	state;
	ostringstream		output;

	auto &topLevelItems = node->children();
	auto it = find_if(topLevelItems.begin(), topLevelItems.end(), entryPointFn);

	if (it != topLevelItems.end())
		state.setCname(*it, (*it)->getName());

	codegen(node, output, state, "");

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
void codegen(Ref<AstNode> node, ostream& output, CodeGeneratorState& state, const string& resultDest)
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
		types[AST_TUPLE_DEF]	= tupleDefCodegen;
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
		types[node->getType()](node, output, state, resultDest);
}

/// <summary>
/// Code generation function for nodes which do not require code generation
/// </summary>
void voidCodegen(Ref<AstNode> node, ostream& output, CodeGeneratorState& state, const string& resultDest)
{
}

/// <summary>
/// Handles the case of node types which are not suppossed to reach code generation phase.
/// </summary>
void invalidNodeCodegen(Ref<AstNode> node, ostream& output, CodeGeneratorState& state, const string& resultDest)
{
	errorAt(node->position(),
		ETYPE_INVALID_CODEGEN_NODE_1,
		astTypeToString(node->getType()).c_str());
}

/// <summary>
/// Calls code generation for all children of the node.
/// </summary>
void nodeListCodegen(Ref<AstNode> node, ostream& output, CodeGeneratorState& state, const string& resultDest)
{
	assert(resultDest == "");

	for (auto child : node->children())
		codegen(child, output, state, "");
}

/// <summary>
/// Generates code for a function definition node.
/// </summary>
void functionCodegen(Ref<AstNode> node, ostream& output, CodeGeneratorState& state, const string& resultDest)
{
	//Generate code for the parameters tuple.
	auto params = node->child(0);
	auto retTuple = node->child(1);
	auto fnCode = node->child(2);
	auto returnType = node->getDataType().staticCast<FunctionType>()->getReturnType();

	if (params->childCount() > 0)
	{
		output << "//Parameters for '" << node->getName() << "' function\n";
		codegen(params, output, state, "");
	}

	if (returnType->type() == DT_TUPLE)
	{
		output << "//Return value for '" << node->getName() << "' function\n";
		codegen(params, output, state, "");
	}

	output << "//Code for '" << node->getName() << "' function\n";
	output << genFunctionHeader(node, state);
	output << "{\n";

	if(returnType->type() == DT_VOID)
		codegen(fnCode, output, state, "");
	else
	{
		TempVariable	tmpReturn(fnCode, output, state);
		codegen(fnCode, output, state, tmpReturn.cname());

		output << "return " << tmpReturn.cname() << ";\n";
	}

	output << "}\n\n";
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
void blockCodegen(Ref<AstNode> node, ostream& output, CodeGeneratorState& state, const string& resultDest)
{
	if (node->childCount() == 0)
		return;

	const auto &	children = node->children();
	auto			lastChild = children.back();

	state.enterBlock();

	output << "{\n";

	for (size_t i = 0; i < children.size() - 1; ++i)
		codegen(children[i], output, state, "");

	codegen(lastChild, output, state, resultDest);

	output << "}\n";

	state.exitBlock();
}

/// <summary>
/// Generates code for a tuple creation expression.
/// </summary>
void tupleCodegen(Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest)
{
	//TODO: Review this check. Is really right not to evaluate the sub-expressions?
	if (resultDest == "")
		return;

	auto	dataType = node->getDataType().staticCast<TupleType>();
	auto&	expressions = node->children();

	for (size_t i = 0; i < expressions.size(); ++i)
	{
		auto childNode = dataType->getMemberNode((unsigned)i);
		string fieldName = state.cname(childNode);
		string childDest = resultDest + "." + fieldName;

		codegen(expressions[i], output, state, resultDest);
	}
}

/// <summary>
/// Generates code for a variable declaration.
/// </summary>
void varCodegen(Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest)
{
	auto	typeNode = node->getDataType();

	output << state.cname(typeNode) << " ";
	output << state.cname(node) << ";\n";
}

/// <summary>
/// Generates code for a tuple definition.
/// </summary>
void tupleDefCodegen(Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest)
{
	output << "struct " << state.cname(node) << "{\n";

	nodeListCodegen(node, output, state, "");

	output << "};\n\n";
}

/// <summary>
/// Generates code for an 'if' flow control expression.
/// </summary>
void ifCodegen(Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest)
{
	auto	condition = node->child(0);
	auto	thenExpr = node->child(1);
	auto	elseExpr = node->child(2);

	//Condition
	TempVariable conditionTempVar(condition, output, state);

	codegen(condition, output, state, conditionTempVar.cname());

	output << "if(" << conditionTempVar.cname() << "){\n";

	//Then
	codegen(thenExpr, output, state, resultDest);
	output << "}\n";

	//Else (optional)
	if (elseExpr.notNull())
	{
		output << "else{\n";
		codegen(elseExpr, output, state, resultDest);
		output << "}\n";
	}
}

/// <summary>
/// Generates code for a return statement
/// </summary>
void returnCodegen(Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest)
{
	if (!node->childExists(0))
		output << "return;\n";
	else
	{
		auto			expression = node->child(0);
		TempVariable	tempVar(expression, output, state);

		codegen(expression, output, state, tempVar.cname());
		output << "return" << tempVar.cname() << ";\n";
	}
}

/// <summary>
/// Generates code for an assignment expression.
/// </summary>
void assignmentCodegen(Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest)
{
	auto	assignment = node.staticCast<AstOperator>();
	auto	lexpr = assignment->child(0);
	auto	rexpr = assignment->child(1);

	assert(assignment->operation == "=");

	if (lexpr->getType() == AST_IDENTIFIER)
	{
		codegen(rexpr, output, state, state.cname(lexpr));
		if (resultDest != "")
			output << resultDest << " = " << state.cname(lexpr);
	}
	else
	{
		//By the moment, only 'member access' nodes are sopported.
		assert(lexpr->getType() == AST_MEMBER_ACCESS);
		TempVariable	tempResult(node, output, state);

		codegen(rexpr, output, state, tempResult.cname());
		memberAccessCodegen(lexpr, output, state, tempResult.cname(), true);

		if (resultDest != "")
			output << resultDest << " = " << tempResult.cname();
	}
}

/// <summary>
/// Generates code for a function call expression.
/// </summary>
void callCodegen(Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest)
{
	auto fnExpr = node->child(0);
	auto paramsExpr = node->child(1);

	//By the moment, only direct function invocation is sopported
	assert(fnExpr->getType() == AST_IDENTIFIER);

	auto	fnNode = node->getScope()->get(fnExpr->getName(), true);
	string	fnCName = state.cname(fnNode);

	if (paramsExpr->childCount() == 0)
	{
		if (resultDest != "")
			output << resultDest << " = ";

		output << fnCName << "();\n";
	}
	else
	{
		TempVariable	tmpParams(paramsExpr, output, state);

		codegen(paramsExpr, output, state, tmpParams.cname());

		if (resultDest != "")
			output << resultDest << " = ";

		output << fnCName << "(&" << tmpParams.cname() << ");\n";
	}
}

/// <summary>
/// Generates code for a literal node
/// </summary>
void literalCodegen(Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest)
{
	if (resultDest == "")
		return;

	switch (node->getType())
	{
	case AST_INTEGER:
	case AST_FLOAT:
		output << resultDest << " = " << node->getValue() << ";\n";
		break;

	case AST_STRING:
		output << resultDest << " = " << escapeString (node->getValue(), true) << ";\n";
		break;

	default:
		assert(!"Unexpected literal type");
	}
}

/// <summary>
/// Generates code to read a variable.
/// </summary>
void varAccessCodegen(Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest)
{
	if (resultDest == "")
		return;

	output << resultDest << " = " << state.cname(node) << ";\n";
}

/// <summary>
/// Generates code for an access to member expression.
/// Just for reads.
/// </summary>
void memberAccessCodegen(Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest)
{
	memberAccessCodegen(node, output, state, resultDest, false);
}

/// <summary>
/// Generates code for an access to member expression.
/// Read / write version.
/// </summary>
/// <param name="node"></param>
/// <param name="output"></param>
/// <param name="state"></param>
/// <param name="varName">'C' name of the variable from which to read the value on writes
/// or to write it on reads.
/// </param>
/// <param name="isWrite">Selects between read and write mode.</param>
void memberAccessCodegen(
	Ref<AstNode> node, 
	std::ostream& output, 
	CodeGeneratorState& state, 
	const std::string& varName, 
	bool isWrite)
{
	auto			left = node->child(0);
	auto			right = node->child(1);
	vector <string>	nameStack;

	nameStack.push_back(state.cname(right));

	do {
		auto type = left->getType();

		if (type == AST_IDENTIFIER)
			nameStack.push_back(state.cname(left));
		else if (type == AST_MEMBER_ACCESS)
		{
			nameStack.push_back(state.cname(left->child(1)));
			left = left->child(0);
		}
		else
		{
			TempVariable	temp(left, output, state);

			codegen(left, output, state, temp.cname());
			nameStack.push_back(temp.cname());
		}
	} while (left->getType() == AST_MEMBER_ACCESS);

	assert(nameStack.size() >= 2);

	if (varName != "")
	{
		std::reverse(nameStack.begin(), nameStack.end());

		if (isWrite)
		{
			output << join(nameStack, ".");
			output << " = " << varName << ";\n";
		}
		else
		{
			output << varName << " = ";
			output << join(nameStack, ".") << ";\n";
		}
	}
}

/// <summary>
/// Generates code for a binary operation
/// </summary>
void binaryOpCodegen(Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest)
{
	if (resultDest == "")
		return;

	auto leftExpr = node->child(0);
	auto rightExpr = node->child(1);
	auto operation = node.staticCast<AstOperator>()->operation;

	TempVariable	leftTmp(leftExpr, output, state);
	TempVariable	rightTmp(rightExpr, output, state);

	codegen(leftExpr, output, state, leftTmp.cname());
	codegen(rightExpr, output, state, rightTmp.cname());

	output << resultDest << " = ";
	output << leftTmp.cname() << operation << rightTmp.cname() << ";\n";
}


/// <summary>
/// Generates code for a prefix operator
/// </summary>
void prefixOpCodegen(Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest)
{
	auto child = node->child(0);
	auto operation = node.staticCast<AstOperator>()->operation;

	TempVariable	temp(child, output, state);

	codegen(child, output, state, temp.cname());

	if (resultDest != "")
		output << resultDest << " = ";
	output << operation << temp.cname() << ";\n";
}


/// <summary>
/// Generates code for a postfix operator
/// </summary>
void postfixOpCodegen(Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest)
{
	auto child = node->child(0);
	auto operation = node.staticCast<AstOperator>()->operation;

	TempVariable	temp(child, output, state);

	codegen(child, output, state, temp.cname());

	output << operation << temp.cname() << ";\n";

	if (resultDest != "")
		output << resultDest << " = " << temp.cname() << ";\n";
}
