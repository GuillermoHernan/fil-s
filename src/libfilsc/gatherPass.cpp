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
		operations.add(AST_CONST, gatherSymbol);
		operations.add(AST_VAR, gatherSymbol);
		operations.add(AST_TYPEDEF, gatherSymbol);
		operations.add(AST_ACTOR, gatherSymbol);

		operations.add(AST_DECLARATION, gatherParameters);
		operations.add(AST_CONST, gatherParameters);
		operations.add(AST_VAR, gatherParameters);
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
	state.rootScope->add("int", astCreateDefaultType("int"));
	state.rootScope->add("bool", astCreateDefaultType("bool"));
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

	return gatherSymbol(node, parent->getScope(), checkParents);
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

	if (!isParameter (state) && name != "")
		return CompileError::ok();
	else
	{
		auto scope = state.parent(1)->getScope();

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
/// Checks whether the current node is function parameter by examining its parents.
/// </summary>
/// <param name="state"></param>
/// <returns></returns>
bool isParameter(const SemAnalysisState& state)
{
	if (state.parent()->getType() != AST_TUPLE_DEF)
		return false;
	else
	{
		auto tuple = state.parent();
		auto fnDef = state.parent(1);

		return (fnDef->getType() == AST_FUNCTION && fnDef->children()[0] == tuple);
	}
}
