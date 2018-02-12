/// <summary>
/// Internal header for 'C' code generator
/// </summary>
#pragma once

#include <functional>
#include "c_codeGenerator.h"

class CodeGeneratorState;

typedef void (*NodeCodegenFN)(Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest);

void codegen(Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest);

void voidCodegen(Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest);
void invalidNodeCodegen(Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest);
void nodeListCodegen (Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest);
void functionCodegen(Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest);
//void typedefCodegen (Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest);
void blockCodegen (Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest);
void tupleCodegen (Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest);
void varCodegen (Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest);
void tupleDefCodegen (Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest);
void ifCodegen (Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest);
void returnCodegen (Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest);
void assignmentCodegen (Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest);
void callCodegen (Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest);
void literalCodegen (Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest);
void varAccessCodegen (Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest);
void memberAccessCodegen(Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest);
void memberAccessCodegen(Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& varName, bool isWrite);
void binaryOpCodegen (Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest);
void prefixOpCodegen (Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest);
void postfixOpCodegen (Ref<AstNode> node, std::ostream& output, CodeGeneratorState& state, const std::string& resultDest);

std::string genFunctionHeader(Ref<AstNode> node, CodeGeneratorState& state);

std::string encodeCString(const std::string& str);