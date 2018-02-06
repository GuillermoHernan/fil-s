#include "pch.h"
#include "gatherPass.h"
#include "semanticAnalysis_internal.h"
#include "SymbolScope.h"
#include "semAnalysisState.h"

using namespace std;

/// <summary>
/// Scans the code for named items (types, variables, functions) to build the symbol table.
/// </summary>
/// <param name="node"></param>
/// <param name="state"></param>
/// <returns></returns>
SemanticResult symbolGatherPass(Ref<AstNode> node, SemAnalysisState& state)
{
	static PassFunctionSet	functions;

	if (functions.empty())
	{
		functions.add(AST_FUNCTION, gatherSymbol);
		functions.add(AST_DECLARATION, gatherSymbol);
		functions.add(AST_CONST, gatherSymbol);
		functions.add(AST_VAR, gatherSymbol);
		functions.add(AST_TYPEDEF, gatherSymbol);
		functions.add(AST_ACTOR, gatherSymbol);

		functions.add(AST_TUPLE_DEF, gatherParameters);
	}
	addDefaultTypes(state);

	return semPreOrderWalk(functions, state, node);
}

/// <summary>
/// Adds default types to the symbol table.
/// </summary>
/// <param name="state"></param>
void addDefaultTypes(SemAnalysisState& state)
{
	state.rootScope->add("int", astCreateDefaultType());
	state.rootScope->add("bool", astCreateDefaultType());
}

/// <summary>
/// Gathers a symbol and adds it to the appropriate scope.
/// </summary>
/// <param name="node"></param>
/// <param name="state"></param>
/// <returns></returns>
CompileError gatherSymbol(Ref<AstNode> node, SemAnalysisState& state)
{
	return gatherSymbol(node, state.parent()->getScope());
}

/// <summary>
/// Gathers a symbol and adds it to the indicated scope.
/// </summary>
/// <param name="node"></param>
/// <param name="state"></param>
/// <returns></returns>
CompileError gatherSymbol(Ref<AstNode> node, Ref<SymbolScope> scope)
{
	string name = node->getName();

	if (name.empty())
		return CompileError::ok();

	if (scope->contains(name))
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
	if (state.parent()->getType() != AST_FUNCTION)
		return CompileError::ok();
	else
	{
		auto scope = state.parent()->getScope();

		for (auto child : node->children())
		{
			auto result = gatherSymbol(child, scope);
			if (!result.isOk())
				return result;
		}

		return CompileError::ok();
	}
}
