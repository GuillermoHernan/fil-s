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
	static PassOperations	symbolFunctions;
	static PassOperations	paramsFunctions;

	if (symbolFunctions.empty())
	{
		symbolFunctions.add(AST_FUNCTION, gatherSymbol);
		symbolFunctions.add(AST_DECLARATION, gatherSymbol);
		symbolFunctions.add(AST_CONST, gatherSymbol);
		symbolFunctions.add(AST_VAR, gatherSymbol);
		symbolFunctions.add(AST_TYPEDEF, gatherSymbol);
		symbolFunctions.add(AST_ACTOR, gatherSymbol);

		paramsFunctions.add(AST_TUPLE_DEF, gatherParameters);
	}
	addDefaultTypes(state);

	auto r1 = semPreOrderWalk(symbolFunctions, state, node);
	auto r2 = semPreOrderWalk(paramsFunctions, state, node);

	return r1.combineWith(r2);
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
