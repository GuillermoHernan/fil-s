/// <summary>
/// Internal header for 'C' code generator
/// </summary>
#pragma once

#include <functional>
#include "c_codeGenerator.h"

class CodeGeneratorState;
class TupleType;

typedef void (*NodeCodegenFN)(Ref<AstNode> node, CodeGeneratorState& state, const std::string& resultDest);

void codegen(Ref<AstNode> node, CodeGeneratorState& state, const std::string& resultDest);

void voidCodegen(Ref<AstNode> node, CodeGeneratorState& state, const std::string& resultDest);
void invalidNodeCodegen(Ref<AstNode> node, CodeGeneratorState& state, const std::string& resultDest);
void nodeListCodegen (Ref<AstNode> node, CodeGeneratorState& state, const std::string& resultDest);
void functionCodegen(Ref<AstNode> node, CodeGeneratorState& state, const std::string& resultDest);
//void typedefCodegen (Ref<AstNode> node, CodeGeneratorState& state, const std::string& resultDest);
void blockCodegen (Ref<AstNode> node, CodeGeneratorState& state, const std::string& resultDest);
void tupleCodegen(Ref<AstNode> node, CodeGeneratorState& state, const std::string& resultDest);
void tupleCodegen(Ref<AstNode> node, CodeGeneratorState& state, const std::string& resultDest, Ref<TupleType> tupleType);
void varCodegen (Ref<AstNode> node, CodeGeneratorState& state, const std::string& resultDest);
void tupleDefCodegen (Ref<AstNode> node, CodeGeneratorState& state, const std::string& resultDest);
void ifCodegen (Ref<AstNode> node, CodeGeneratorState& state, const std::string& resultDest);
void returnCodegen (Ref<AstNode> node, CodeGeneratorState& state, const std::string& resultDest);
void assignmentCodegen (Ref<AstNode> node, CodeGeneratorState& state, const std::string& resultDest);
void callCodegen (Ref<AstNode> node, CodeGeneratorState& state, const std::string& resultDest);
void literalCodegen (Ref<AstNode> node, CodeGeneratorState& state, const std::string& resultDest);
void varAccessCodegen (Ref<AstNode> node, CodeGeneratorState& state, const std::string& resultDest);
void memberAccessCodegen(Ref<AstNode> node, CodeGeneratorState& state, const std::string& resultDest);
void memberAccessCodegen(Ref<AstNode> node, CodeGeneratorState& state, const std::string& varName, bool isWrite);
void binaryOpCodegen (Ref<AstNode> node, CodeGeneratorState& state, const std::string& resultDest);
void prefixOpCodegen (Ref<AstNode> node, CodeGeneratorState& state, const std::string& resultDest);
void postfixOpCodegen (Ref<AstNode> node, CodeGeneratorState& state, const std::string& resultDest);

std::string genFunctionHeader(Ref<AstNode> node, CodeGeneratorState& state);
