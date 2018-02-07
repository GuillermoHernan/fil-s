#include "pch.h"
#include "typeCheckPass.h"
#include "semanticAnalysis_internal.h"
#include "symbolScope.h"

using namespace std;

/// <summary>
/// This pass checks that type references are valid.
/// </summary>
/// <param name="node"></param>
/// <param name="state"></param>
/// <returns></returns>
SemanticResult typeCheckPass(Ref<AstNode> node, SemAnalysisState& state)
{
	static PassFunctionSet	functions;

	if (functions.empty())
	{
		functions.add(AST_TYPE_NAME, typeExistsCheck);
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
/// Checks whether an AST node represents a type or not.
/// </summary>
/// <param name="node"></param>
/// <returns></returns>
bool isType(Ref<AstNode> node)
{
	AstNodeTypes type = node->getType();

	return type == AST_DEFAULT_TYPE
		|| type == AST_TUPLE_DEF
		|| type == AST_ACTOR;
}
