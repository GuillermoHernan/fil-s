/// <summary>
/// Helper class to organize the operations (functions) that are executed in a 
/// compiler pass.
/// </summary>

#include "pch.h"
#include "passOperations.h"
#include "semanticAnalysis_internal.h"

using namespace std;

/// <summary>
/// Adds a 'check' function to the set.
/// </summary>
/// <param name="type"></param>
/// <param name="checkFn"></param>
void PassOperations::add(AstNodeTypes type, CheckFunction checkFn)
{
	if (m_checkFunctions.count(type) == 0)
		m_checkFunctions[type] = CheckFnList();

	m_checkFunctions[type].push_back(checkFn);
}

/// <summary>
/// Adds a 'transform' function to the set.
/// </summary>
/// <param name="type"></param>
/// <param name="transformFn"></param>
void PassOperations::add(AstNodeTypes type, TransformFunction transformFn)
{
	if (m_transformFunctions.count(type) == 0)
		m_transformFunctions[type] = TransformFnList();

	m_transformFunctions[type].push_back(transformFn);
}

/// <summary>
/// Checks if set is empty
/// </summary>
/// <returns></returns>
bool PassOperations::empty()const
{
	return m_checkFunctions.empty() && m_transformFunctions.empty();
}

/// <summary>Calls all functions on a node.</summary>
/// <remarks>If any check is not passed, transform functions are not executed.</remarks>
/// <param name="node"></param>
/// <param name="state"></param>
/// <returns></returns>
SemanticResult PassOperations::processNode(Ref<AstNode> node, SemAnalysisState& state)const
{
	vector<CompileError>	errors;
	auto					itCheck = m_checkFunctions.find(node->getType());

	//Perform checks
	if (itCheck != m_checkFunctions.end())
	{
		for (auto checkFn : itCheck->second)
		{
			auto err = checkFn(node, state);

			if (!err.isOk())
				errors.push_back(err);
		}
	}

	if (!errors.empty())
		return SemanticResult(errors);

	//Perform transformations if no errors have been found.
	auto itTransform = m_transformFunctions.find(node->getType());

	if (itTransform != m_transformFunctions.end())
	{
		for (auto transformFn : itTransform->second)
			node = transformFn(node, state);
	}

	return SemanticResult(node);
}
