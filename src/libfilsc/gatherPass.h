/// <summary>
/// Compiler pass which looks for program symbols and adds them to the scope.
/// </summary>

#pragma once

#include "semanticAnalysis.h"

class SemAnalysisState;
class SymbolScope;

SemanticResult symbolGatherPass(Ref<AstNode> node, SemAnalysisState& state);

void addDefaultTypes(SemAnalysisState& state);
CompileError gatherSymbol(Ref<AstNode> node, SemAnalysisState& state);
CompileError gatherSymbol(Ref<AstNode> node, Ref<SymbolScope> scope, bool checkParents);
CompileError gatherParameters(Ref<AstNode> node, SemAnalysisState& state);
CompileError importSymbols(Ref<AstNode> node, SemAnalysisState& state);
CompileError importSymbols(const std::string& modName, Ref<AstNode> targetNode, SemAnalysisState& state);
CompileError importRuntime(Ref<AstNode> node, SemAnalysisState& state);

Ref<AstNode> defaultToConst(Ref<AstNode> node, SemAnalysisState& state);

