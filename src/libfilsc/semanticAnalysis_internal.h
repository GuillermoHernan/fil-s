/// <summary>
/// Semantic analizer internal header.
/// Contains the declarations of the functions and data types used internally by the semantic analyzer.
/// </summary>

#pragma once

#include "semanticAnalysis.h"
#include <functional>

class SemAnalysisState;
class PassOperations;
class SymbolScope;

typedef std::function <SemanticResult(Ref<AstNode>, SemAnalysisState&)> PassFunction;
//typedef std::function <Ref<AstNode> (Ref<AstNode>, SemAnalysisState&)> TransformFunction;
//typedef std::function <CompileError (Ref<AstNode>, SemAnalysisState&)> CheckFunction;

typedef std::vector<PassFunction>					PassList;

const PassList& getSemAnalysisPasses();

SemanticResult semInOrderWalk(const PassOperations& fnSet, SemAnalysisState& state, Ref<AstNode> node);
SemanticResult semInOrderWalk(PassFunction fn, SemAnalysisState& state, Ref<AstNode> node);

SemanticResult semPreOrderWalk(const PassOperations& fnSet, SemAnalysisState& state, Ref<AstNode> node);
SemanticResult semPreOrderWalk(PassFunction fn, SemAnalysisState& state, Ref<AstNode> node);

CompileError semError(Ref<AstNode> node, ErrorTypes type, ...);

SemanticResult buildModuleNode(const AstStr2NodesMap& nodes, const std::string& name);

Ref<AstNode> createUnnamedTypesNode(const SemAnalysisState& state);


