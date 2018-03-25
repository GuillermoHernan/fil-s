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

using namespace std;

/// <summary>
/// Builds a module, from its sources on the filesystem.
/// </summary>
/// <param name="moduleDir"></param>
/// <returns></returns>
BuildResult buildModule(const std::string& modulePath, const std::string& builderPath)
{
    StrSet		parents;
    auto		result = getDependencies(modulePath, parents);

    if (!result.ok())
        return BuildResult(result.errors);

    //Add dependency to runtime.
    auto frt = findRuntime(builderPath);
    if (frt == nullptr)
    {
        auto error = CompileError::create(ScriptPosition(), ETYPE_CANNOT_FIND_RUNTIME);
        return BuildResult(error);
    }

    return buildWithDependencies(result.result.get(), builderPath);
}

/// <summary>
/// Obtains the dependency tree of a module.
/// </summary>
/// <param name="modulePath">Path of the module in the file system</param>
/// <param name="parents">Set of paranet modules path to prevent circular references</param>
/// <returns></returns>
DependenciesResult getDependencies(const std::string& modulePath, StrSet& parents)
{
    try
    {
        preventCircularReferences(modulePath, parents);

        DepencencyTreePtr	node(new ModuleNode(modulePath));

        auto parseRes = parseSourceFiles(node.get());
        if (!parseRes.ok())
            return DependenciesResult(parseRes.errors);

        auto depResult = getDependentModules(node.get());
        auto errorList = depResult.errors;
        auto childModules = depResult.result;

        parents.insert(modulePath);
        for (auto& childPath : childModules)
        {
            auto childResult = getDependencies(childPath, parents);

            if (childResult.ok())
                node->addDependency(move(childResult.result));
            else
                childResult.appendErrorsTo(errorList);
        }
        parents.erase(modulePath);

        if (errorList.empty())
            return DependenciesResult(move(node));
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
/// Builds a module, checking first its dependencies.
/// </summary>
/// <param name="module"></param>
/// <returns></returns>
BuildResult buildWithDependencies(ModuleNode* module, const std::string& builderPath)
{
    //TODO: This does not take into account the if the dependency modules are updated to 
    //update the build of the current module.
    if (module->buildNeeded())
    {
        vector<CompileError>	errors;

        //First, check that all dependencies are up-to-date.
        module->walkDependencies([&errors, builderPath](ModuleNode* dependency) {
            //TODO: Optimization oportunity: The build of each module can be done in parallel.
            auto r = buildWithDependencies(dependency, builderPath);

            if (!r.ok())
                r.appendErrorsTo(errors);
        });

        if (!errors.empty())
            return BuildResult(errors);


        return buildModule(module, builderPath);
    }

    return SuccessfulResult(true);
}

/// <summary>
/// Performs a module build, once their dependencies are up-to-date.
/// </summary>
/// <param name="module"></param>
/// <returns></returns>
BuildResult	buildModule(ModuleNode* module, const std::string& builderPath)
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

    auto r = semanticAnalysis(sources, modules);

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
        return buildExecutable(module, builderPath);
    else
        return SuccessfulResult(true);

}

/// <summary>
/// Looks for FIL-S runtime module in builder path
/// </summary>
/// <param name="builderPath"></param>
/// <returns></returns>
DepencencyTreePtr findRuntime(const std::string& builderPath)
{
    string path = findModuleInDir("frt", fs::path(builderPath));

    if (path == "")
        return nullptr;
    else
        return DepencencyTreePtr (new ModuleNode(path));
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
/// <returns></returns>
OperationResult<StrList> getDependentModules(ModuleNode* module)
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
        auto resolveRes = resolveModuleName(module->path(), modRef.first, modRef.second);

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
    const NodeSet& refNodes)
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

    StrList		systemLibPaths = getSystemLibPaths();

    for (auto& libPath : systemLibPaths)
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
/// Gets the default library paths.
/// </summary>
/// <returns></returns>
StrList getSystemLibPaths()
{
    static StrList	paths;

    if (paths.empty())
    {
        const char* varContent = getenv("FILS_LIBPATHS");

        if (varContent != nullptr)
            paths = split(varContent, ";");
    }

    return paths;
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

    return any_of(nodes.begin(), nodes.end(), isEntryPoint);
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
BuildResult buildExecutable(ModuleNode* module, const std::string& builderPath)
{
    assert(!module->buildNeeded());

    try
    {
        string code = generateCode(module->getAST(), isEntryPoint);

        writeCCodeFile(code, module);
        _flushall();	//To ensure all generated files are written to the disk.

        return compileC(module, builderPath);
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
/// <param name="module"></param>
/// <returns></returns>
BuildResult compileC(ModuleNode* module, const std::string& builderPath)
{
    string scriptPath = createCompileScript(module, builderPath);
    string command = getCompileScriptCommand(module, builderPath);

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
/// Creates the script which is used to invoke the external 'C' compiler.
/// </summary>
/// <remarks>
/// The compilation script is based on a template which is installed along with
/// FIL-S compiler/builder. This allows to change the compiler / compiler options
/// easily.
/// </remarks>
/// <param name="module">Module for which the script is going to be generated.</param>
/// <returns>It returns the path of the created script on the filesystem</returns>
std::string createCompileScript(ModuleNode* module, const std::string& builderPath)
{
    string scriptTemplate = getCompileScriptTemplate(builderPath);
    string script = replaceScriptVariables(scriptTemplate, module);

    auto lines = split(script, "\n");
    if (lines.size() < 3)
    {
        throw CompileError::create(ScriptPosition(),
            ETYPE_INVALID_COMPILE_SCRIPT_TEMPLATE_1,
            "It should have, at least, 3 lines");
    }

    //First line is the script name
    //Second line, the command to execute the script.
    //The script begins at the third line.
    string fileName = trim(lines[0]);
    string path = joinPaths(module->path(), fileName);

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
std::string getCompileScriptCommand(ModuleNode* module, const std::string& builderPath)
{
    string scriptTemplate = getCompileScriptTemplate(builderPath);

    auto lines = split(scriptTemplate, "\n");
    if (lines.size() < 3)
    {
        throw CompileError::create(ScriptPosition(),
            ETYPE_INVALID_COMPILE_SCRIPT_TEMPLATE_1,
            "It should have, at least, 3 lines");
    }

    //The script invokation command is at the second line of the script template.
    return replaceScriptVariables(lines[1], module);
}

/// <summary>
/// Gets the compilation script template.
/// </summary>
/// <remarks>
/// The compilation script is based on a template which is installed along with
/// FIL-S compiler/builder. This allows to change the compiler / compiler options
/// easily.
/// </remarks>
/// <param name="builderPath">Path where the builder executable is located, to find the template</param>
/// <returns>Template contents in a std::string</returns>
std::string getCompileScriptTemplate(const std::string& builderPath)
{
    string path = joinPaths(builderPath, "c_compile_script.tmpl");

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
/// <param name="module"></param>
/// <returns></returns>
std::string	replaceScriptVariables(const std::string& scriptTemplate, ModuleNode* module)
{
    map<string, string>		internalVars;

    internalVars["ModulePath"] = module->path();
    internalVars["ModuleName"] = module->name();

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
    size_t begin = scriptTemplate.find("${");

    if (begin == string::npos)
        return FindVariableResult("", string::npos);

    size_t	end = scriptTemplate.find_first_of("{}", begin + 2);
    if (end == string::npos)
        return FindVariableResult("", string::npos);
    else if (scriptTemplate[end] == '{')
        return findScriptVariable(scriptTemplate, end + 1);
    else
        return FindVariableResult(scriptTemplate.substr(begin + 2, end - (begin + 2)), begin);

}
