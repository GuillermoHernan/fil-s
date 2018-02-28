#pragma once

#include "ast.h"
#include "semanticAnalysis.h"

class SemAnalysisState;
class CompileError;

/// <summary>
/// Helper class to organize the operations (functions) that are executed in a 
/// compiler pass.
/// </summary>
/// <remarks>
/// * All applicable operations are executed on each node of the tree.
/// * Node type is used to determine which are the applicable operations.
/// * There are two types of operations: 'Check' and 'Transform'.
/// * 'Check' operations perform checks on the node, and return a compile error 
/// if some compiler rule has been broken.
/// * 'Transform' operations can replace a node by other node.
/// * If any check is not passed on a node, 'Transform' operations are no executed.
/// </remarks>
class PassOperations
{
public:
	typedef CompileError(*CheckFunction)(Ref<AstNode> node, SemAnalysisState& state);
	typedef std::vector<CheckFunction>					CheckFnList;

	typedef Ref<AstNode>(*TransformFunction)(Ref<AstNode> node, SemAnalysisState& state);
	typedef std::vector<TransformFunction>				TransformFnList;


	void add(AstNodeTypes type, CheckFunction checkFn);
	void add(AstNodeTypes type, TransformFunction transformFn);

	bool empty()const;

	SemanticResult processNode(Ref<AstNode> node, SemAnalysisState& state)const;

private:
	std::map<AstNodeTypes, CheckFnList>		m_checkFunctions;
	std::map<AstNodeTypes, TransformFnList>	m_transformFunctions;
};

