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
	SemanticResult		result(node);
	SemAnalysisState	state;

	for (auto pass : passes)
	{
		result = pass(node, state);

		if (!result.ok())
			return result;
		else
			node = result.ast;
	}

	return result;
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
				node->setChild(i, result.ast);
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
		node = result.ast;

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
				node->setChild(i, childResult.ast);
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

/// <summary>Combines two semantic results. </summary>
/// <remarks>
/// * If any of the two result contains errors, it yields a result which combines both error lists.
/// * If both are ok, the resulting AST node is from result 'r2'
/// </remarks>
/// <param name="r2"></param>
/// <returns></returns>
SemanticResult SemanticResult::combineWith(const SemanticResult& r2)const
{
	if (!this->ok())
	{
		std::vector<CompileError>	newErrList = this->errors;

		newErrList.insert(newErrList.end(), r2.errors.begin(), r2.errors.end());
		return SemanticResult(newErrList);
	}
	else
		return r2;
}
