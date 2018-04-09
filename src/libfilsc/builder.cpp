/// <summary>
/// Builder: Builds FIL-S modules from its sources and dependencies.
/// </summary>

#include "pch.h"
#include "builder_internal.h"
#include "semanticAnalysis.h"
#include "c_codeGenerator.h"
#include "compileError.h"
//#include "ast.h"
#include "parser.h"
#include "utils.h"
#include "dependencySolver.h"

using namespace std;

/// <summary>
/// Builds a module, from its sources on the filesystem.
/// </summary>
/// <param name="moduleDir"></param>
/// <param name="cfg">Builder configuration structure.</param>
/// <returns></returns>
BuildResult buildModule(const std::string& modulePath, const BuilderConfig& cfg)
{
    auto checkResult = checkConfig(cfg);

    if (!checkResult.ok())
        return BuildResult(checkResult.errors);

    auto & cfgOk = checkResult.result;

    StrSet		parents;
    ModuleMap   modules;
    auto		depResult = getDependencies(modulePath, modules, parents, cfgOk);

    if (!depResult.ok())
        return BuildResult(depResult.errors);

    //Dependency sort.
    auto                rootModule = depResult.result;
    vector<ModuleNode*> modList;
    modList.push_back(rootModule.get());

    modList = dependencySort<ModuleNode*>(modList, [](auto mod) {
        set<ModuleNode*>  moduleSet;
        mod->walkDependencies([&moduleSet](auto childModule) {
            moduleSet.insert(childModule);
        });
        return moduleSet;
    });

    //build modules in dependency order
    for (auto module : modList)
    {
        //TODO: Optimization oportunity: The build of each module can be done in parallel,
        //as long as its dependencies are built.
        auto r = buildModule(module, cfgOk);

        //It stops at he first module which fails, because it is potentially needed
        //by the next modules in the list.
        if (!r.ok())
            return r;
    }

    return BuildResult(rootModule.get());
}

/// <summary>
/// Checks and completes the supplied configuration.
/// </summary>
/// <param name="cfg"></param>
/// <returns></returns>
OperationResult<BuilderConfig> checkConfig(const BuilderConfig& cfg)
{
    typedef OperationResult<BuilderConfig>  ResultType;

    BuilderConfig   newCfg = cfg;

    if (newCfg.BasePath.empty())
    {
        auto error = CompileError::create(ScriptPosition(), ETYPE_BASE_DIR_NOT_CONFIGURED);
        return ResultType(error);
    }

    if (newCfg.RuntimePath.empty())
    {
        //Look for runtime lib.
        newCfg.RuntimePath = findRuntime(cfg.BasePath);
        if (newCfg.RuntimePath.empty())
        {
            auto error = CompileError::create(ScriptPosition(), ETYPE_CANNOT_FIND_RUNTIME);
            return ResultType(error);
        }
    }

    //TODO: By the moment, this is the only available platform
    if (newCfg.PlatformName.empty())
        newCfg.PlatformName = "Win32Sim";

    if (newCfg.PlatformPath.empty())
    {
        newCfg.PlatformPath = joinPaths(cfg.BasePath, "platforms/" + newCfg.PlatformName);
        newCfg.PlatformPath = normalizePath(newCfg.PlatformPath);
    }

    if (newCfg.LibPaths.empty())
    {
        newCfg.LibPaths = getSystemLibPaths();
        newCfg.LibPaths.push_back(newCfg.BasePath);
        newCfg.LibPaths.push_back(newCfg.PlatformPath);
    }

    return ResultType(move(newCfg));
}

/// <summary>
/// Gets the default library paths.
/// </summary>
/// <returns></returns>
StrList getSystemLibPaths()
{
    StrList	paths;

    const char* varContent = getenv("FILS_LIBPATHS");

    if (varContent != nullptr)
        paths = split(varContent, ";");

    return paths;
}


/// <summary>
/// Obtains the dependency tree of a module.
/// </summary>
/// <param name="modulePath">Path of the module in the file system</param>
/// <param name="modules">Cache of already loaded modules, to load them only once.</param>
/// <param name="parents">Set of parent modules path to prevent circular references</param>
/// <param name="runtimePath">Runtime library path. All modules depend of this library.</param>
/// <returns></returns>
DependenciesResult getDependencies(
    const std::string& modulePath,
    ModuleMap& modules,
    StrSet& parents,
    const BuilderConfig& cfg)
{
    try
    {
        auto itModule = modules.find(modulePath);

        if (itModule != modules.end())
            return DependenciesResult(itModule->second);        //Modules cache hit.

        preventCircularReferences(modulePath, parents);

        ModuleNodePtr	node(new ModuleNode(modulePath));
        modules[modulePath] = node;

        auto parseRes = parseSourceFiles(node.get());
        if (!parseRes.ok())
            return DependenciesResult(parseRes.errors);

        auto depResult = getDependentModules(node.get(), cfg);
        auto errorList = depResult.errors;
        auto childModules = depResult.result;

        //All modules depend on runtime, except for itself, and the modules on which FRT depends.
        parents.insert(modulePath);
        if (parents.count(cfg.RuntimePath) == 0)
            childModules.push_back(cfg.RuntimePath);

        for (auto& childPath : childModules)
        {
            auto childResult = getDependencies(childPath, modules, parents, cfg);

            if (childResult.ok())
                node->addDependency(childResult.result);
            else
                childResult.appendErrorsTo(errorList);
        }
        parents.erase(modulePath);

        if (errorList.empty())
            return DependenciesResult(node);
        else
            return DependenciesResult(errorList);
    }
    catch (const CompileError& error)
    {
        parents.erase(modulePath);			//Just in case
        return DependenciesResult(error);
    }
}

/// <summary>
/// Performs a module build, once their dependencies are up-to-date.
/// </summary>
/// <param name="module"></param>
/// <returns></returns>
BuildResult	buildModule(ModuleNode* module, const BuilderConfig& cfg)
{
    AstStr2NodesMap		modules;
    AstStr2NodesMap		sources;

    //Create the map which maps module names to its AST, needed to perform 
    //semantic analysis.
    module->walkDependencies([&modules](auto dependency) {
        modules[dependency->name()] = dependency->getAST();
    });

    //Fill map of (source path)->(parsed AST)
    module->walkSources([&sources](auto srcFile) {
        sources[srcFile->path()] = srcFile->getAST();
    });

    auto r = semanticAnalysis(module->name(), sources, modules);

    if (!r.ok())
        return BuildResult(r.errors);

    try
    {
        module->setAST(r.result);
    }
    catch (const exception& error)
    {
        return BuildResult(CompileError::create(
            ScriptPosition(),
            ETYPE_WRITING_RESULT_FILE_2,
            module->getCompiledPath().c_str(),
            error.what()));
    }

    if (containsEntryPoint(r.result))
        return buildExecutable(module, cfg);
    else
        return SuccessfulResult(true);

}

/// <summary>
/// Looks for FIL-S runtime module in builder path
/// </summary>
/// <param name="builderPath"></param>
/// <returns></returns>
std::string findRuntime(const std::string& builderPath)
{
    return findModuleInDir("frt", fs::path(builderPath));
}

/// <summary>
/// It ensures that the files belonging to the node (module) have been 
/// parsed. If some file is not parsed, it parses it.
/// </summary>
/// <remarks>
/// Parsing means that it is just parsed, it does not perform semantic analysis.
/// </remarks>
/// <param name="node"></param>
/// <returns></returns>
OperationResult<bool> parseSourceFiles(ModuleNode* module)
{
    typedef OperationResult<bool> RetType;

    if (!module->buildNeeded())
        return RetType(true);
    else
    {
        vector<CompileError>	errors;

        //TODO: Optimization oportunity. Each file is parsed independently, so
        //it can be done in parallel.
        module->walkSources([&errors](auto file) {
            auto parseRes = parseFile(file->ref());

            if (parseRes.ok())
                file->setAST(parseRes.result);
            else
                errors.push_back(parseRes.errorDesc);
        });

        if (errors.empty())
            return RetType(true);
        else
            return RetType(errors);
    }
}

/// <summary>
/// Gets the list of dependent modules from of a module.
/// </summary>
/// <param name="module"></param>
/// <returns>List of module paths</returns>
OperationResult<StrList> getDependentModules(ModuleNode* module, const BuilderConfig& cfg)
{
    ModuleRefsMap   modReferences;

    if (!module->buildNeeded())
        scanImports(module->getAST(), &modReferences);
    else
    {
        module->walkSources([&modReferences](auto file) {
            scanImports(file->getAST(), &modReferences);
        });
    }

    vector<CompileError>	errors;
    StrList					modulePaths;
    for (auto& modRef : modReferences)
    {
        auto resolveRes = resolveModuleName(module->path(), modRef.first, modRef.second, cfg);

        if (resolveRes.ok())
            modulePaths.push_back(resolveRes.result);
        else
            resolveRes.appendErrorsTo(errors);
    }

    if (errors.empty())
        return SuccessfulResult(modulePaths);
    else
    {
        OperationResult<StrList>	failure(errors);

        //In this case, we also return the dependencies sucessfully found.
        failure.result = modulePaths;
        return failure;
    }
}

/// <summary>
/// Checks for any circular reference detected in module imports.
/// If detected, it throws a 'CompileError' exception.
/// </summary>
/// <param name="modulePath"></param>
/// <param name="parents"></param>
void preventCircularReferences(const std::string& modulePath, StrSet& parents)
{
    //TODO: circular reference errors should have location.
    if (parents.count(modulePath) > 0)
        errorAt(ScriptPosition(), ETYPE_CIRCULAR_MODULE_REFERENCE_1, modulePath.c_str());
}


/// <summary>
/// Scans an AST node for 'import' statements.
/// </summary>
/// <param name="ast"></param>
/// <param name="moduleRefs">Module references map. 
/// Maps (Module name) => (Set of 'import' nodes which reference it)</param>
void scanImports(Ref<AstNode> ast, ModuleRefsMap* moduleRefs)
{
    //Only module nodes are scanned recursively.
    for (auto node : ast->children())
    {
        if (node.notNull())
        {
            switch (node->getType())
            {
            case AST_IMPORT:
                if (!node->hasFlag(ASTF_EXTERN_C))
                    (*moduleRefs)[node->getValue()].insert(node.getPointer());
                break;

            case AST_MODULE:
                scanImports(node, moduleRefs);
                break;

            default:
                break;
            }//switch
        }//if
    }//for
}

/// <summary>
/// Given a module name, and a base path, it tries to find the path of the module.
/// </summary>
/// <param name="basePath"></param>
/// <param name="moduleName"></param>
/// <param name="refNodes">Nodes from which the module is referenced. To create compile errors.</param>
/// <returns></returns>
OperationResult<std::string> resolveModuleName(
    const std::string& basePath,
    const std::string& moduleName,
    const NodeSet& refNodes,
    const BuilderConfig& cfg)
{
    assert(!refNodes.empty());

    fs::path	base(basePath);
    string		result;

    //Current module directory.
    result = findModuleInDir(moduleName, base);
    if (!result.empty())
        return SuccessfulResult(result);

    //Current module parent directory
    result = findModuleInDir(moduleName, base.parent_path());
    if (!result.empty())
        return SuccessfulResult(result);

    for (auto& libPath : cfg.LibPaths)
    {
        result = findModuleInDir(moduleName, libPath);
        if (!result.empty())
            return SuccessfulResult(result);
    }

    //Create a compile error on each node which references the module.
    vector<CompileError>    errors;
    for (auto node : refNodes)
    {
        auto error = CompileError::create(
            node->position(), 
            ETYPE_MODULE_NOT_FOUND_1, 
            moduleName.c_str());

        errors.push_back(error);
    }

    return OperationResult<std::string>(errors);
}

//Looks for a module in a given directory.
std::string findModuleInDir(const std::string& moduleName, const fs::path& directory)
{
    fs::path modulePath = directory / moduleName;

    if (isModuleDirectory(modulePath))
        return modulePath.u8string();
    else
    {
        error_code	ec;

        modulePath = directory / (moduleName + ".fast");
        if (fs::is_regular_file(fs::status(modulePath, ec)))
            return modulePath.u8string();
        else
            return "";
    }
}

/// <summary>
/// Checks if a given path is a FILS module directory
/// </summary>
/// <param name="modulePath"></param>
/// <returns></returns>
bool isModuleDirectory(const fs::path& modulePath)
{
    //Check if the path exists and it is a directory
    error_code	ec;

    if (!fs::is_directory(fs::status(modulePath, ec)))
        return false;

    auto isSourceFile = [](fs::path p) {
        return p.extension() == ".fil";
    };

    return any_of(fs::directory_iterator(modulePath), fs::directory_iterator(), isSourceFile);
}

/// <summary>
/// Checks if a compiled module contains the official entry point
/// (actor '_Main')
/// </summary>
/// <param name="ast"></param>
/// <returns></returns>
bool containsEntryPoint(Ref<AstNode> ast)
{
    const auto & nodes = ast->children();

    switch (ast->getType())
    {
    case AST_MODULE:
        return any_of(nodes.begin(), nodes.end(), containsEntryPoint);

    case AST_SCRIPT:
        return any_of(nodes.begin(), nodes.end(), isEntryPoint);

    default:
        return false;
    }
}

/// <summary>
/// Checks whether an AST node is the entry point.
/// </summary>
/// <param name="node"></param>
/// <returns></returns>
bool isEntryPoint(Ref<AstNode> node)
{
    return node->getType() == AST_ACTOR && node->getName() == "_Main";
}


/// <summary>
/// Generates code from a successfully compiled module.
/// </summary>
/// <param name="module"></param>
/// <returns></returns>
BuildResult buildExecutable(ModuleNode* module, const BuilderConfig& cfg)
{
    //TODO: re-enable this assert.
    //assert(!module->buildNeeded());

    try
    {
        string code = generateCode(module->getAST(), isEntryPoint);

        writeCCodeFile(code, module);
        _flushall();	//To ensure all generated files are written to the disk.

        auto deps = getCLibrariesDependencies(module, cfg);

        if (!deps.ok())
            return BuildResult(deps.errors);
        else
            return compileC(module, deps.result, cfg);
    }
    catch (const CompileError & error)
    {
        return BuildResult(error);
    }
}

/// <summary>
/// Writes to the filesystem the generated 'C' code, in the appropriate file.
/// </summary>
/// <param name="code">Contains the full 'C' code</param>
/// <param name="module">Module information</param>
void writeCCodeFile(const std::string& code, ModuleNode* module)
{
    if (!writeTextFile(module->getCFilePath(), code))
    {
        throw CompileError::create(ScriptPosition(),
            ETYPE_WRITING_RESULT_FILE_2,
            module->getCFilePath().c_str(),
            "Cannot write to file"
        );
    }
}

/// <summary>
/// Compiles 'C' code, invoking an external compiler.
/// </summary>
/// <param name="module">Root module to build.</param>
/// <param name="cLibraries">'C' libraries dependencies. In binary format (.lib, .a)
/// It is map which maps each library name to its path.</param>
/// <param name="cfg">Builder configuration</param>
/// <returns></returns>
BuildResult compileC(ModuleNode* module, const StrMap& cLibraries, const BuilderConfig& cfg)
{
    string scriptPath = createCompileScript(module, cLibraries, cfg);
    string command = getCompileScriptCommand(module, cfg);

    _flushall();
    int result = system(command.c_str());
    if (result != 0)
    {
        return BuildResult(CompileError::create(ScriptPosition(),
            ETYPE_ERROR_COMPILING_C_1,
            module->path().c_str()));
    }
    else
        return BuildResult(true);
}

/// <summary>
/// Gets all 'C' libraries dependencies of a module.
/// </summary>
/// <param name="module"></param>
/// <param name="cfg"></param>
/// <returns></returns>
OperationResult<StrMap> getCLibrariesDependencies(ModuleNode* module, const BuilderConfig& cfg)
{
    StrMap                  libraries;
    vector<CompileError>    errors;

    //TODO: Handle the case of a library with same name, but different location. Error?
    //TODO: Múltiple errors if module appears several times in the dependency graph. That is
    //legal. Circular references are illegal.

    //Check child modules.
    module->walkDependencies([&libraries, &errors, &cfg](auto childModule) {
        auto r = getCLibrariesDependencies(childModule, cfg);

        if (r.ok())
            libraries.insert(r.result.begin(), r.result.end());
        else
            r.appendErrorsTo(errors);
    });

    auto importNodes = getCImports(module);

    for (auto node : importNodes)
    {
        string name = node->getValue();

        if (libraries.count(name) == 0)
        {
            string path = findCLibrary(name, module, cfg);

            if (path.empty())
            {
                auto error = CompileError::create(node->position(),
                    ETYPE_C_LIBRARY_NOT_FOUND_1, 
                    externCLibraryFilename(name).c_str());
                errors.push_back(error);
            }
            else
                libraries[name] = path;
        }
    }

    if (errors.empty())
        return OperationResult<StrMap>(libraries);
    else
        return OperationResult<StrMap>(errors);
}

/// <summary>
/// Finds 'import[C]' statements in a module.
/// </summary>
/// <param name="module"></param>
/// <returns></returns>
AstNodeList getCImports(ModuleNode* module)
{
    auto            ast = module->getAST();
    AstNodeList     result;

    assert(ast.notNull());

    for (auto script : ast->children())
    {
        if (script->getType() == AST_SCRIPT)
        {
            for (auto node : script->children())
            {
                if (node->getType() == AST_IMPORT && node->hasFlag(ASTF_EXTERN_C))
                    result.push_back(node);
            }
        }
    }//for

    return result;
}

/// <summary>
/// Looks for a 'C' library imported by a module.
/// </summary>
/// <param name="name"></param>
/// <param name="module"></param>
/// <param name="cfg"></param>
/// <returns></returns>
std::string findCLibrary(const std::string& name, ModuleNode* module, const BuilderConfig& cfg)
{
    //TODO: The logic which defines the order in which directories are examined is copied
    //from function 'resolveModuleName'. Look for a way to share this logic.

    fs::path	base(module->path());
    string      fileName = externCLibraryFilename(name);
    string		result;
    error_code	ec;

    //Current module directory.
    fs::path    libPath = base / fileName;
    if (fs::is_regular_file(fs::status(libPath, ec)))
        return libPath.u8string();

    //Current module parent directory
    libPath = base.parent_path() / fileName;
    if (fs::is_regular_file(fs::status(libPath, ec)))
        return libPath.u8string();

    //Configured library paths.
    for (auto& p : cfg.LibPaths)
    {
        libPath = fs::path(p) / fileName;
        if (fs::is_regular_file(fs::status(libPath, ec)))
            return libPath.u8string();
    }

    return "";
}

/// <summary>
/// Gets the file name of an external 'C' library, given its name.
/// </summary>
/// <param name="name"></param>
/// <returns></returns>
std::string externCLibraryFilename(const std::string& name)
{
    //TODO: May be, this should be controlled by a configurable parameter, instead of
    //a define, dependent of the compiler host architecture.
#ifdef _WIN32
    return name + ".lib";
#else
    return "lib" + name + ".a";
#endif
}


/// <summary>
/// Creates the script which is used to invoke the external 'C' compiler.
/// </summary>
/// <remarks>
/// The compilation script is based on a template which is installed along with
/// FIL-S compiler/builder. This allows to change the compiler / compiler options
/// easily.
/// </remarks>
/// <param name="module">Module for which the script is going to be generated.</param>
/// <returns>It returns the path of the created script on the filesystem</returns>
std::string createCompileScript(
    ModuleNode* module, 
    const StrMap& cLibraries, 
    const BuilderConfig& cfg)
{
    string scriptTemplate = getCompileScriptTemplate(cfg);
    string script = replaceScriptVariables(scriptTemplate, cLibraries, module);

    auto lines = split(script, "\n");
    if (lines.size() < 3)
    {
        throw CompileError::create(ScriptPosition(),
            ETYPE_INVALID_COMPILE_SCRIPT_TEMPLATE_1,
            "It should have, at least, 3 lines");
    }

    //Handle Windows end of lines, if present.
    for (auto& line : lines)
        line = trim(line, "\r");

    //First line is the script name
    //Second line, the command to execute the script.
    //The script begins at the third line.
    string fileName = trim(lines[0]);
    string path = joinPaths(module->getIntermediateDir(), fileName);

    script = join(lines, "\n", 2);
    if (!writeTextFile(path, script))
    {
        throw CompileError::create(ScriptPosition(),
            ETYPE_WRITING_RESULT_FILE_2,
            module->getCFilePath().c_str(),
            "Cannot write to file"
        );
    }

    //module->setCompileScriptPath(path);
    return path;
}

/// <summary>
/// Gets the command used to invoke the compile script
/// </summary>
/// <param name="module"></param>
/// <returns></returns>
std::string getCompileScriptCommand(ModuleNode* module, const BuilderConfig& cfg)
{
    string scriptTemplate = getCompileScriptTemplate(cfg);

    auto lines = split(scriptTemplate, "\n");
    if (lines.size() < 3)
    {
        throw CompileError::create(ScriptPosition(),
            ETYPE_INVALID_COMPILE_SCRIPT_TEMPLATE_1,
            "It should have, at least, 3 lines");
    }

    //The script invokation command is at the second line of the script template.
    return replaceScriptVariables(lines[1], StrMap(), module);
}

/// <summary>
/// Gets the compilation script template.
/// </summary>
/// <remarks>
/// The compilation script is based on a template which is installed along with
/// FIL-S compiler/builder. This allows to change the compiler / compiler options
/// easily.
/// </remarks>
/// <param name="cfg">Build system configuration</param>
/// <returns>Template contents in a std::string</returns>
std::string getCompileScriptTemplate(const BuilderConfig& cfg)
{
    string path = joinPaths(cfg.PlatformPath, "c_compile_template.tmpl");

    string content = readTextFile(path);

    if (content == "")
    {
        throw CompileError::create(ScriptPosition(),
            ETYPE_COMPILE_SCRIPT_TEMPLATE_NOT_FOUND_1,
            path.c_str());
    }
    else
        return content;
}

/// <summary>
/// Replaces variables in the script template, in order to generate the 'C' compile script.
/// </summary>
/// <param name="scriptTemplate"></param>
/// <param name="cLibraries"></param>
/// <param name="module"></param>
/// <returns></returns>
std::string	replaceScriptVariables(
    const std::string& scriptTemplate, 
    const StrMap& cLibraries,
    ModuleNode* module)
{
    map<string, string>		internalVars;
    set<string>             libPathsSet;
    vector<string>          libraries;

    for (auto& libEntry : cLibraries)
    {
        fs::path    libPath(libEntry.second);
        libPath = libPath.parent_path();

        libPathsSet.insert("\"" + libPath.u8string() + "\"");

        string name = externCLibraryFilename(libEntry.first);
        libraries.push_back("\"" + name + "\"");
    }
    vector<string> libPaths (libPathsSet.begin(), libPathsSet.end());

    internalVars["ModulePath"] = module->path();
    internalVars["ModuleName"] = module->name();
    internalVars["CFilePath"] = module->getCFilePath();
    internalVars["IntermediateDir"] = module->getIntermediateDir();
    internalVars["BinDir"] = module->getBinDir();
    internalVars["LibPaths"] = join(libPaths, ",");
    internalVars["LibNames"] = join(libraries, ",");

    string	result;
    size_t	curPosition = 0;

    result.reserve((scriptTemplate.size() * 12) / 10);

    auto varInfo = findScriptVariable(scriptTemplate, 0);

    while (varInfo.found())
    {
        result += scriptTemplate.substr(curPosition, varInfo.begin() - curPosition);

        string	text = varInfo.text();

        if (!text.empty())
        {
            if (!isAlpha(text[0]))
                result += text;
            else
            {
                auto	itVar = internalVars.find(varInfo.text());

                if (itVar != internalVars.end())
                    result += itVar->second;
                else
                {
                    const char *envValue = getenv(text.c_str());

                    if (envValue != NULL)
                        result += envValue;
                    else
                        result += text + "_not_found";
                }
            }
        }//if (!text.empty)

        curPosition = varInfo.end();
        varInfo = findScriptVariable(scriptTemplate, curPosition);
    }//while

    result += scriptTemplate.substr(curPosition);

    return result;
}

/// <summary>
/// Finds a variable inside the compile script template
/// </summary>
/// <param name="scriptTemplate"></param>
/// <param name="initialPosition"></param>
/// <returns></returns>
FindVariableResult findScriptVariable(const std::string& scriptTemplate, size_t initialPosition)
{
    size_t begin = scriptTemplate.find("${", initialPosition);

    if (begin == string::npos)
        return FindVariableResult("", string::npos);

    size_t	end = scriptTemplate.find_first_of("{}", begin + 2);
    if (end == string::npos)
        return FindVariableResult("", string::npos);
    else if (scriptTemplate[end] == '{')
        return findScriptVariable(scriptTemplate, end - 1);
    else
        return FindVariableResult(scriptTemplate.substr(begin + 2, end - (begin + 2)), begin);

}
