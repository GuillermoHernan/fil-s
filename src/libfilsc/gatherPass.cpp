#include "pch.h"
#include "gatherPass.h"
#include "semanticAnalysis_internal.h"
#include "SymbolScope.h"
#include "semAnalysisState.h"
#include "passOperations.h"

using namespace std;

/// <summary>
/// Scans the code for named items (types, variables, functions) to build the symbol table.
/// </summary>
/// <param name="node"></param>
/// <param name="state"></param>
/// <returns></returns>
SemanticResult symbolGatherPass(Ref<AstNode> node, SemAnalysisState& state)
{
	static PassOperations	operations;

	if (operations.empty())
	{
		operations.add(AST_FUNCTION, gatherSymbol);
		operations.add(AST_DECLARATION, gatherSymbol);
		operations.add(AST_TYPEDEF, gatherSymbol);
		operations.add(AST_ACTOR, gatherSymbol);
		operations.add(AST_INPUT, gatherSymbol);
		operations.add(AST_OUTPUT, gatherSymbol);

		operations.add(AST_DECLARATION, gatherParameters);
		operations.add(AST_DECLARATION, defaultToConst);
	}
	addDefaultTypes(state);

	return semPreOrderWalk(operations, state, node);
}

/// <summary>
/// Adds default types to the symbol table.
/// </summary>
/// <param name="state"></param>
void addDefaultTypes(SemAnalysisState& state)
{
	state.rootScope->add("int", astGetInt());
	state.rootScope->add("bool", astGetBool());
}

/// <summary>
/// Gathers a symbol and adds it to the appropriate scope.
/// </summary>
/// <param name="node"></param>
/// <param name="state"></param>
/// <returns></returns>
CompileError gatherSymbol(Ref<AstNode> node, SemAnalysisState& state)
{
	auto parent = state.parent();
	const bool checkParents = parent->getType() != AST_TUPLE_DEF;

	return gatherSymbol(node, state.getScope(parent), checkParents);
}

/// <summary>
/// Gathers a symbol and adds it to the indicated scope.
/// </summary>
/// <param name="node"></param>
/// <param name="state"></param>
/// <returns></returns>
CompileError gatherSymbol(Ref<AstNode> node, Ref<SymbolScope> scope, bool checkParents)
{
	string name = node->getName();

	if (name.empty())
		return CompileError::ok();

	if (scope->contains(name, checkParents))
		return semError(node, ETYPE_SYMBOL_ALREADY_DEFINED_1, name.c_str());
	else
	{
		scope->add(name, node);
		return CompileError::ok();
	}
}

/// <summary>
/// Gathers the parameter definitions from a tuple and injects them in the scope of
/// the containing function.
/// </summary>
/// <param name="node"></param>
/// <param name="state"></param>
/// <returns></returns>
CompileError gatherParameters(Ref<AstNode> node, SemAnalysisState& state)
{
	string name = node->getName();

	if (!node->hasFlag(ASTF_FUNCTION_PARAMETER) || name == "")
		return CompileError::ok();
	else
	{
		auto scope = state.getScope(state.parent(1));

		if (scope->contains(name, true))
			return semError(node, ETYPE_SYMBOL_ALREADY_DEFINED_1, name.c_str());
		else
		{
			scope->add(name, node);
			return CompileError::ok();
		}
	}
}

/// <summary>
/// Defaults to 'const' any declaration for has no access specifier (var/const)
/// </summary>
/// <param name="node"></param>
/// <param name="state"></param>
/// <returns></returns>
Ref<AstNode> defaultToConst(Ref<AstNode> node, SemAnalysisState& state)
{
	if (!node->hasFlag(ASTF_VAR))
		node->addFlag(ASTF_CONST);

	return node;
}