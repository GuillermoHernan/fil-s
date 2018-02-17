/// <summary>
/// Compiler pass which checks that data types are correctly used.
/// </summary>
#pragma once

#include "ast.h"
class SemanticResult;
class SemAnalysisState;
class CompileError;
class BaseType;
class TupleType;

SemanticResult typeCheckPass(Ref<AstNode> node, SemAnalysisState& state);
SemanticResult preTypeCheckPass(Ref<AstNode> node, SemAnalysisState& state);

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
CompileError literalTypeAssign(Ref<AstNode> node, SemAnalysisState& state);
CompileError defaultTypeAssign(Ref<AstNode> node, SemAnalysisState& state);

Ref<AstNode> tupleRemoveTypedef(Ref<AstNode> node, SemAnalysisState& state);

CompileError setVoidType(Ref<AstNode> node, SemAnalysisState& state);

CompileError	areTypesCompatible(Ref<BaseType> typeA, Ref<BaseType> typeB, Ref<AstNode> opNode);
bool			areTypesCompatible(Ref<BaseType> typeA, Ref<BaseType> typeB);
bool			areTuplesCompatible(Ref<TupleType> tupleA, Ref<TupleType> tupleB);
Ref<BaseType>	getCommonType(Ref<BaseType> typeA, Ref<BaseType> typeB, SemAnalysisState& state);

bool isBoolType(Ref<BaseType> type);
bool isIntType(Ref<BaseType> type);
bool isVoidType(Ref<BaseType> type);


bool isType(Ref<AstNode> node);

