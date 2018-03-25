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
        operations.add(AST_SCRIPT, importRuntime);
        operations.add(AST_IMPORT, importSymbols);
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
/// Imports the symbols from another module.
/// Effectively implements 'import' sentence.
/// </summary>
/// <param name="node"></param>
/// <param name="state"></param>
/// <returns></returns>
CompileError importSymbols(Ref<AstNode> node, SemAnalysisState& state)
{
    if (!node->hasFlag(ASTF_EXTERN_C))
        return importSymbols(node->getValue(), state.parent(), state);
    else
        return CompileError::ok();
}

/// <summary>
/// Imports the symbols from another referenced module.
/// </summary>
/// <param name="modName"></param>
/// <param name="targetNode"></param>
/// <param name="state"></param>
/// <returns></returns>
CompileError importSymbols(const std::string& modName, Ref<AstNode> targetNode, SemAnalysisState& state)
{
    auto scope = state.getScope(targetNode);

    auto itMod = state.modules.find(modName);
    assert(itMod != state.modules.end());

    for (auto item : itMod->second->children())
    {
        if (!item->getName().empty() && item->getName()[0] != '_')
            scope->add(item->getName(), item);
    }

    return CompileError::ok();
}

/// <summary>
/// Imports basic runtime module, if it is present among the dependencies.
/// It is executed on 'AST_SCRIPT' nodes.
/// </summary>
/// <param name="node"></param>
/// <param name="state"></param>
/// <returns></returns>
CompileError importRuntime(Ref<AstNode> node, SemAnalysisState& state)
{
    if (state.modules.count("frt") > 0)
        return importSymbols("frt", node, state);
    else
        return CompileError::ok();
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