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
//useful, and this an internal header, which is included from few source files.


typedef OperationResult<std::shared_ptr<ModuleNode>>    DependenciesResult;
typedef std::vector< std::string >				        StrList;
typedef std::set< std::string >						    StrSet;
typedef std::map<std::string, std::string>              StrMap;
typedef std::set< AstNode* >				    		NodeSet;
typedef std::map<std::string, NodeSet>                  ModuleRefsMap;

OperationResult<BuilderConfig>  checkConfig(const BuilderConfig& cfg);
StrList						    getSystemLibPaths();

DependenciesResult			getDependencies(
    const std::string& modulePath, 
    ModuleMap& modules,
    StrSet& parents,
    const BuilderConfig& cfg);
BuildResult					buildModule(ModuleNode* module, const BuilderConfig& cfg);

std::string                 findRuntime(const std::string& builderPath);

BuildResult					parseSourceFiles(ModuleNode* module);
OperationResult<StrList>	getDependentModules(ModuleNode* module, const BuilderConfig& cfg);
void						preventCircularReferences(const std::string& modulePath, StrSet& parents);

void						scanImports(Ref<AstNode> ast, ModuleRefsMap* moduleRefs);
OperationResult<std::string> resolveModuleName(
    const std::string& basePath, 
    const std::string& moduleName,
    const NodeSet& refNodes,
    const BuilderConfig& cfg);

std::string					findModuleInDir(const std::string& moduleName, const fs::path& directory);
bool						isModuleDirectory(const fs::path& modulePath);

bool						containsEntryPoint(Ref<AstNode> ast);
bool						isEntryPoint(Ref<AstNode> node);

BuildResult					buildExecutable(ModuleNode* module, const BuilderConfig& cfg);
void						writeCCodeFile(const std::string& code, ModuleNode* module);
BuildResult					compileC(ModuleNode* module, const StrMap& cLibraries, const BuilderConfig& cfg);

OperationResult<StrMap>     getCLibrariesDependencies(ModuleNode* module, const BuilderConfig& cfg);
AstNodeList                 getCImports(ModuleNode* module);
std::string                 findCLibrary(
    const std::string& name, 
    ModuleNode* module, 
    const BuilderConfig& cfg);
std::string                 externCLibraryFilename(const std::string& name);

std::string					createCompileScript(ModuleNode* module, const StrMap& cLibraries, const BuilderConfig& cfg);
std::string					getCompileScriptCommand(ModuleNode* module, const BuilderConfig& cfg);
std::string					getCompileScriptTemplate(const BuilderConfig& cfg);
std::string					replaceScriptVariables(
    const std::string& scriptTemplate, 
    const StrMap& cLibraries, 
    ModuleNode* module);

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
