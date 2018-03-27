/// <summary>
/// Internal header for 'C' code generator
/// </summary>
#pragma once

#include <functional>
#include "c_codeGenerator.h"

class CodeGeneratorState;
struct IVariableInfo;

typedef void(*NodeCodegenFN)(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);

void writeProlog(std::ostream& output);
void codegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);

void dataTypeCodegen(AstNode* type, CodeGeneratorState& state);

void voidCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void invalidNodeCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void nodeListCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void moduleCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void declareFunction(AstNode* node, CodeGeneratorState& state);
void functionCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
//void typedefCodegen (Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void blockCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void tupleCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void varCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void tupleDefCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void tupleDefCodegen(AstNode* type, CodeGeneratorState& state);
void tupleAdapterCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void ifCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void returnCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void assignmentCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void callCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void literalCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void varAccessCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void memberAccessCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void binaryOpCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void prefixOpCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void postfixOpCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);

void actorCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void outputMessageCodegen(Ref<AstNode> node, CodeGeneratorState& state, const IVariableInfo& resultDest);
void generateActorStruct(AstNode* type, CodeGeneratorState& state);
void generateActorConstructor(Ref<AstNode> node, CodeGeneratorState& state);
void generateActorInputs(Ref<AstNode> node, CodeGeneratorState& state);
void generateActorInput(Ref<AstNode> actor, Ref<AstNode> input, CodeGeneratorState& state);
void generateConnection(Ref<AstNode> actor, Ref<AstNode> connection, CodeGeneratorState& state);

std::string genFunctionHeader(Ref<AstNode> node, CodeGeneratorState& state);
std::string genInputMsgHeader(Ref<AstNode> actor,
    Ref<AstNode> input,
    CodeGeneratorState& state,
    const std::string& nameOverride = "");
void generateParamsStruct(Ref<AstNode> node, CodeGeneratorState& state, const std::string& commentSufix);
std::string varAccessExpression(Ref<AstNode> node, CodeGeneratorState& state);
