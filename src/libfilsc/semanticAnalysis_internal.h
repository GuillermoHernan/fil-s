/// <summary>
/// Semantic analizer internal header.
/// Contains the declarations of the functions and data types used internally by the semantic analyzer.
/// </summary>

#pragma once

#include "semanticAnalysis.h"

class SemAnalysisState;
class PassFunctionSet;
class SymbolScope;

typedef SemanticResult(*PassFunction)(Ref<AstNode> node, SemAnalysisState& state);
typedef Ref<AstNode>(*TransformFunction)(Ref<AstNode> node, SemAnalysisState& state);
typedef CompileError(*CheckFunction)(Ref<AstNode> node, SemAnalysisState& state);

typedef std::vector<PassFunction>					PassList;
typedef std::vector<CheckFunction>					CheckFnList;
typedef std::vector<TransformFunction>				TransformFnList;

const PassList& getSemAnalysisPasses();

SemanticResult modulesPass(Ref<AstNode> node, SemAnalysisState& state);

SemanticResult semInOrderWalk(const PassFunctionSet& fnSet, SemAnalysisState& state, Ref<AstNode> node);
SemanticResult semPreOrderWalk(const PassFunctionSet& fnSet, SemAnalysisState& state, Ref<AstNode> node);

CompileError semError(Ref<AstNode> node, ErrorTypes type, ...);

/// <summary>
/// Stores the functions (check or transform) which are used on a Semantic analysis pass.
/// </summary>
class PassFunctionSet
{
public:
	void add(AstNodeTypes type, CheckFunction checkFn);
	void add(AstNodeTypes type, TransformFunction transformFn);

	bool empty()const;

	SemanticResult processNode(Ref<AstNode> node, SemAnalysisState& state)const;

private:
	std::map<AstNodeTypes, CheckFnList>		m_checkFunctions;
	std::map<AstNodeTypes, TransformFnList>	m_transformFunctions;
};

//Gather pass.
