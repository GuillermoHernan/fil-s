/// <summary>
/// Functions to join the ASTs from several source files into a single AST.
///
/// Part of the build system.
/// </summary>

#pragma once

#include "operationResult.h"
#include "ast.h"
#include <map>

typedef OperationResult<Ref<AstNode>> AssemblyResult;

AssemblyResult assembleModule(const std::string& moduleName, const AstNodeList& sources);
AssemblyResult assignImportedModules(Ref<AstNode> moduleNode, const AstStr2NodesMap& modules);
