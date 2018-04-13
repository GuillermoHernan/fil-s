/// <summary>
/// Generates 'C' language code from the checked program AST.
/// </summary>
/// <remarks>
/// It does not try to perform much optimization, as the 'C' compiler is suppossed 
/// to be good optimizing code.
/// </remarks>
#pragma once

#include "ast.h"
#include <string>
#include <map>

/// <summary>
/// Struture which contains configuration parameters of code generator.
/// </summary>
struct CodeGeneratorConfig
{
    //Symbols which have predefined names in generated 'C' code.
    std::map<std::string, std::string>  predefNames;

    //Prolog and epilog to be added to genrated 'C' source.
    std::string     prolog;
    std::string     epilog;
};

std::string generateCode(Ref<AstNode> node);
std::string generateCode(Ref<AstNode> node, const CodeGeneratorConfig& config);
