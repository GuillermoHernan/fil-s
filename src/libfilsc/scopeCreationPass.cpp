/// <summary>
/// Pass of the semantic analysis phase in which the scope hierarchy is created
/// and assigned to the AST nodes.
/// </summary>

#include "pch.h"
#include "scopeCreationPass.h"
#include "semanticAnalysis_internal.h"
#include "SymbolScope.h"
#include "semAnalysisState.h"


bool needsOwnScope(Ref<AstNode> node);
void buildScope(Ref<AstNode> node, Ref<SymbolScope> currentScope, SemAnalysisState& state);

/// <summary>
/// 'Pass' function which performs scope creation.
/// </summary>
/// <param name="node"></param>
/// <param name="state"></param>
/// <returns></returns>
SemanticResult scopeCreationPass(Ref<AstNode> node, SemAnalysisState& state)
{
    buildScope(node, state.rootScope, state);

    return SemanticResult(node);
}

/// <summary>
/// Recursive function which performs scope creation.
/// </summary>
/// <param name="node"></param>
/// <param name="currentScope"></param>
void buildScope(Ref<AstNode> node, Ref<SymbolScope> currentScope, SemAnalysisState& state)
{
    if (node.isNull())
        return;

    if (needsOwnScope(node))
        currentScope = SymbolScope::create(currentScope);

    state.setScope(node, currentScope);

    //Walk children
    for (auto child : node->children())
    {
        if (node->getType() == AST_MODULE && child->getType() != AST_SCRIPT)
            break;

        buildScope(child, currentScope, state);
    }
}

/// <summary>
/// Checks if the node requires a new scope for itself
/// </summary>
/// <param name="node"></param>
/// <returns></returns>
bool needsOwnScope(Ref<AstNode> node)
{
    AstNodeTypes	type = node->getType();

    return type == AST_BLOCK
        || type == AST_FOR
        || type == AST_TUPLE_DEF
        || type == AST_FUNCTION
        || type == AST_INPUT
        || type == AST_ACTOR
        || type == AST_UNNAMED_INPUT
        || type == AST_SCRIPT
        ;
}
