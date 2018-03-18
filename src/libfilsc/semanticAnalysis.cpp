#include "pch.h"
#include "semanticAnalysis_internal.h"
#include "SymbolScope.h"
#include "semAnalysisState.h"
#include "passOperations.h"

#include "scopeCreationPass.h"
#include "gatherPass.h"
#include "typeCheckPass.h"


using namespace std;

/// <summary>
/// Entry point for semantic analysis.
/// </summary>
/// <param name="node"></param>
/// <returns></returns>
SemanticResult semanticAnalysis(Ref<AstNode> node)
{
    const auto &		passes = getSemAnalysisPasses();
    SemAnalysisState	state;

    for (auto pass : passes)
    {
        auto result = pass(node, state);

        if (!result.ok())
            return result;
        else
            node = result.result;
    }

    return SemanticResult(node);
}

/// <summary>
/// Semantic analysis entry point, when modules are used.
/// </summary>
/// <param name="sources">Map of source file names to its parsed ASTs. These ASTs are
/// parsed, but not semantically analyzed.</param>
/// <param name="modules">Compiled modules map. These are the modules on which the current 
/// module depends. These ASTs are already semantically analyzed.</param>
/// <returns></returns>
SemanticResult semanticAnalysis(const AstStr2NodesMap& sources, const AstStr2NodesMap& modules)
{
    const auto &		passes = getSemAnalysisPasses();
    SemAnalysisState	state;
    AstStr2NodesMap		resultNodes = sources;

    state.modules = modules;

    for (auto pass : passes)
    {
        SemanticResult::ErrorList	errors;

        for (auto& src : sources)
        {
            state.currentFile = src.first;
            auto result = pass(resultNodes[src.first], state);

            if (!result.ok())
                result.appendErrorsTo(errors);
            else
                resultNodes[src.first] = result.result;
        }

        if (!errors.empty())
            return SemanticResult(errors);
    }

    return buildModuleNode(resultNodes);
}


/// <summary>
/// Gets the list of semantic analysis passes to execute.
/// The list is in execution ordenr
/// </summary>
/// <returns></returns>
const PassList& getSemAnalysisPasses()
{
    static PassList passes;

    if (passes.empty())
    {
        passes.push_back(modulesPass);
        passes.push_back(scopeCreationPass);
        passes.push_back(symbolGatherPass);
        passes.push_back(preTypeCheckPass);
        passes.push_back(typeCheckPass);
        passes.push_back(typeCheckPass2);
    }

    return passes;
}

/// <summary>
/// Gathers referenced modules, and builds them if necessary.
/// </summary>
/// <param name="node"></param>
/// <param name="state"></param>
/// <returns></returns>
SemanticResult modulesPass(Ref<AstNode> node, SemAnalysisState& state)
{
    //TODO: implement it.
    return SemanticResult(node);
}

/// <summary>
/// Walks AST in order (leaf nodes first)
/// </summary>
/// <param name="fnSet"></param>
/// <param name="state"></param>
/// <param name="node"></param>
/// <returns></returns>
SemanticResult semInOrderWalk(const PassOperations& fnSet, SemAnalysisState& state, Ref<AstNode> node)
{
    return semInOrderWalk([&fnSet](Ref<AstNode> node, SemAnalysisState& state) {
        return fnSet.processNode(node, state);
    }, state, node);
}

/// <summary>
/// Walks AST in order (leaf nodes first)
/// </summary>
/// <param name="fn"></param>
/// <param name="state"></param>
/// <param name="node"></param>
/// <returns></returns>
SemanticResult semInOrderWalk(PassFunction fn, SemAnalysisState& state, Ref<AstNode> node)
{
    auto&						children = node->children();
    std::vector<CompileError>	errors;

    state.pushParent(node);
    for (size_t i = 0; i < children.size(); ++i)
    {
        auto child = children[i];

        if (child.notNull())
        {
            auto result = semInOrderWalk(fn, state, child);

            if (!result.ok())
                errors.insert(errors.end(), result.errors.begin(), result.errors.end());
            else
                node->setChild(i, result.result);
        }
    }
    state.popParent();

    auto result = fn(node, state);

    if (!errors.empty() || !result.ok())
    {
        errors.insert(errors.end(), result.errors.begin(), result.errors.end());
        return SemanticResult(errors);
    }
    else
        return result;
}

/// <summary>
/// Walks AST in pre-order (root nodes first)
/// </summary>
/// <param name="fnSet"></param>
/// <param name="state"></param>
/// <param name="node"></param>
/// <returns></returns>
SemanticResult semPreOrderWalk(const PassOperations& fnSet, SemAnalysisState& state, Ref<AstNode> node)
{
    return semPreOrderWalk([&fnSet](Ref<AstNode> node, SemAnalysisState& state) {
        return fnSet.processNode(node, state);
    }, state, node);
}

/// <summary>
/// Walks AST in pre-order (root nodes first)
/// </summary>
/// <param name="fnSet"></param>
/// <param name="state"></param>
/// <param name="node"></param>
/// <returns></returns>
SemanticResult semPreOrderWalk(PassFunction fn, SemAnalysisState& state, Ref<AstNode> node)
{
    auto&						children = node->children();
    std::vector<CompileError>	errors;

    auto result = fn(node, state);

    if (!result.ok())
        errors.insert(errors.end(), result.errors.begin(), result.errors.end());
    else
        node = result.result;

    state.pushParent(node);
    //Walk children after root
    for (size_t i = 0; i < children.size(); ++i)
    {
        auto child = children[i];

        if (child.notNull())
        {
            auto childResult = semPreOrderWalk(fn, state, child);

            if (!childResult.ok())
                errors.insert(errors.end(), childResult.errors.begin(), childResult.errors.end());
            else
                node->setChild(i, childResult.result);
        }
    }
    state.popParent();

    if (!errors.empty())
        return SemanticResult(errors);
    else
        return result;
}

/// <summary>
/// Creates a semantic analysis error
/// </summary>
/// <param name="node"></param>
/// <param name="type"></param>
/// <param name=""></param>
/// <returns></returns>
CompileError semError(Ref<AstNode> node, ErrorTypes type, ...)
{
    va_list aptr;

    va_start(aptr, type);
    CompileError result = CompileError::create(node->position(), type, aptr);
    va_end(aptr);

    return result;
}

/// <summary>
/// Builds an 'AST_MODULE' node from the node map.
/// </summary>
/// <param name="nodes"></param>
/// <returns></returns>
SemanticResult buildModuleNode(const AstStr2NodesMap& nodes)
{
    //TODO: set real module name
    auto moduleNode = astCreateModule("");

    for (auto& nodeEntry : nodes)
        moduleNode->addChild(nodeEntry.second);

    return SemanticResult(moduleNode);
}
