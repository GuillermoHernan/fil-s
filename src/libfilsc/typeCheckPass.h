/// <summary>
/// Compiler pass which checks that data types are correctly used.
/// </summary>
#pragma once

#include "semanticAnalysis.h"

class SemAnalysisState;
class CompileError;

SemanticResult preTypeCheckPass(Ref<AstNode> node, SemAnalysisState& state);
SemanticResult typeCheckPass(Ref<AstNode> node, SemAnalysisState& state);
SemanticResult typeCheckPass2(Ref<AstNode> node, SemAnalysisState& state);

CompileError recursiveSymbolReferenceCheck(Ref<AstNode> node, SemAnalysisState& state);

CompileError typeExistsCheck(Ref<AstNode> node, SemAnalysisState& state);
CompileError tupleDefTypeCheck(Ref<AstNode> node, SemAnalysisState& state);

CompileError blockTypeCheck(Ref<AstNode> node, SemAnalysisState& state);
CompileError typedefTypeCheck(Ref<AstNode> node, SemAnalysisState& state);
CompileError tupleTypeCheck(Ref<AstNode> node, SemAnalysisState& state);
CompileError declarationTypeCheck(Ref<AstNode> node, SemAnalysisState& state);
CompileError ifTypeCheck(Ref<AstNode> node, SemAnalysisState& state);
CompileError functionDefTypeCheck(Ref<AstNode> node, SemAnalysisState& state);
CompileError assignmentTypeCheck(Ref<AstNode> node, SemAnalysisState& state);
CompileError callTypeCheck(Ref<AstNode> node, SemAnalysisState& state);
CompileError varReadTypeCheck(Ref<AstNode> node, SemAnalysisState& state);
CompileError memberAccessTypeCheck(Ref<AstNode> node, SemAnalysisState& state);
CompileError binaryOpTypeCheck(Ref<AstNode> node, SemAnalysisState& state);
CompileError prefixOpTypeCheck(Ref<AstNode> node, SemAnalysisState& state);
CompileError postfixOpTypeCheck(Ref<AstNode> node, SemAnalysisState& state);

CompileError mathOperatorTypeCheck(Ref<AstNode> node, SemAnalysisState& state);
CompileError bitwiseOperatorTypeCheck(Ref<AstNode> node, SemAnalysisState& state);
CompileError comparisionOperatorTypeCheck(Ref<AstNode> node, SemAnalysisState& state);
CompileError equalityOperatorTypeCheck(Ref<AstNode> node, SemAnalysisState& state);
CompileError logicalOperatorTypeCheck(Ref<AstNode> node, SemAnalysisState& state);


CompileError returnTypeAssign(Ref<AstNode> node, SemAnalysisState& state);
CompileError returnTypeCheck(Ref<AstNode> node, SemAnalysisState& state);
CompileError literalTypeAssign(Ref<AstNode> node, SemAnalysisState& state);
CompileError defaultTypeAssign(Ref<AstNode> node, SemAnalysisState& state);

CompileError actorTypeCheck(Ref<AstNode> node, SemAnalysisState& state);
CompileError assignItselftAsType(Ref<AstNode> node, SemAnalysisState& state);
CompileError inputMessageTypeCheck(Ref<AstNode> node, SemAnalysisState& state);
CompileError unnamedInputTypeCheck(Ref<AstNode> node, SemAnalysisState& state);
CompileError actorInstanceTypeCheck(Ref<AstNode> node, SemAnalysisState& state);
AstNode* getConnectOutputType(Ref<AstNode> pathNode, SemAnalysisState& state);


Ref<AstNode> tupleRemoveTypedef(Ref<AstNode> node, SemAnalysisState& state);
Ref<AstNode> addTupleAdapter(Ref<AstNode> node, SemAnalysisState& state);
Ref<AstNode> makeTupleAdapter(Ref<AstNode> rNode, AstNode* lType);
Ref<AstNode> addReturnTupleAdapter(Ref<AstNode> node, SemAnalysisState& state);

CompileError setVoidType(Ref<AstNode> node, SemAnalysisState& state);

CompileError	areTypesCompatible(AstNode* typeA, AstNode* typeB, Ref<AstNode> opNode);
bool			areTypesCompatible(AstNode* typeA, AstNode* typeB);
bool			areTuplesCompatible(AstNode* typeA, AstNode* typeB);
bool            areFunctionTypesCompatible(AstNode* typeA, AstNode* typeB);
AstNode*		getCommonType(AstNode* typeA, AstNode* typeB, SemAnalysisState& state);
//Ref<AstNode> buildTupleDefFromTupleType(Ref<TupleType> tuple, const ScriptPosition& pos);

bool isType(Ref<AstNode> node);

