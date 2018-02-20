/// <summary>
/// Internal header for 'C' code generator
/// </summary>
#pragma once

#include <functional>
#include "c_codeGenerator.h"

class CodeGeneratorState;
class TupleType;
struct IVariableInfo;

typedef void(*NodeCodegenFN)(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);

void codegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);

void voidCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void invalidNodeCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void nodeListCodegen (Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void functionCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
//void typedefCodegen (Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void blockCodegen (Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void tupleCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void varCodegen (Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void tupleDefCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void tupleDefCodegen(Ref<BaseType> type, CodeGeneratorState& state);
void tupleAdapterCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void ifCodegen (Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void returnCodegen (Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void assignmentCodegen (Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void callCodegen (Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void literalCodegen (Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void varAccessCodegen (Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void memberAccessCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void memberAccessCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& variable, bool isWrite);
void binaryOpCodegen (Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void prefixOpCodegen (Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void postfixOpCodegen (Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);

std::string genFunctionHeader(Ref<AstNode> node, CodeGeneratorState& state);
std::string varAccessExpression(Ref<AstNode> node, CodeGeneratorState& state);
