/// <summary>
/// Internal functions of the build system.
/// This header is mainly intended for unit test purposes.
/// </summary>

#pragma once
#include "builder.h"
#include "DependencyTree.h"
#include <filesystem>


//TODO: filesystem namespace is supossed to be already in 'std' namespace.
//Change as appropiate of put an '#ifdef' to handle different compilers / standard library versions.
namespace fs = std::experimental::filesystem;
//I know that putting 'using namespace' in headers is not good practice, but in this case was quite
//useful, and this an internal header, which is included from very feww source files.


typedef OperationResult<std::shared_ptr<ModuleNode>>    DependenciesResult;
typedef std::vector< std::string >				        StrList;
typedef std::set< std::string >						    StrSet;
typedef std::set< AstNode* >				    		NodeSet;
typedef std::map<std::string, NodeSet>                  ModuleRefsMap;

DependenciesResult			getDependencies(
    const std::string& modulePath, 
    ModuleMap& modules,
    StrSet& parents,
    const std::string& runtimePath);
BuildResult					buildWithDependencies(ModuleNode* module, const std::string& builderPath);
BuildResult					buildModule(ModuleNode* module, const std::string& builderPath);

std::string                 findRuntime(const std::string& builderPath);

BuildResult					parseSourceFiles(ModuleNode* module);
OperationResult<StrList>	getDependentModules(ModuleNode* module);
void						preventCircularReferences(const std::string& modulePath, StrSet& parents);

void						scanImports(Ref<AstNode> ast, ModuleRefsMap* moduleRefs);
OperationResult<std::string> resolveModuleName(
    const std::string& basePath, 
    const std::string& moduleName,
    const NodeSet& refNodes);

std::string					findModuleInDir(const std::string& moduleName, const fs::path& directory);
bool						isModuleDirectory(const fs::path& modulePath);
StrList						getSystemLibPaths();

bool						containsEntryPoint(Ref<AstNode> ast);
bool						isEntryPoint(Ref<AstNode> node);

BuildResult					buildExecutable(ModuleNode* module, const std::string& builderPath);
void						writeCCodeFile(const std::string& code, ModuleNode* module);
BuildResult					compileC(ModuleNode* module, const std::string& builderPath);

std::string					createCompileScript(ModuleNode* module, const std::string& builderPath);
std::string					getCompileScriptCommand(ModuleNode* module, const std::string& builderPath);
std::string					getCompileScriptTemplate(const std::string& builderPath);
std::string					replaceScriptVariables(const std::string& scriptTemplate, ModuleNode* module);

/// <summary>
/// Result for a 'findScriptVariable' operation.
/// Several queries can be made to this result. Therefore, it is quite useful that it
/// has its own class.
/// </summary>
class FindVariableResult
{
public:
    FindVariableResult(const std::string& text, size_t beginPosition)
        : m_text(text), m_begin(beginPosition)
    {}

    bool		found()const { return m_begin != std::string::npos; }
    std::string	text()const { return m_text; }
    size_t		begin()const { return m_begin; }
    size_t		end()const { return m_begin + m_text.size() + 3; }

private:
    std::string		m_text;
    size_t			m_begin;
};

FindVariableResult			findScriptVariable(const std::string& scriptTemplate, size_t initialPosition);
