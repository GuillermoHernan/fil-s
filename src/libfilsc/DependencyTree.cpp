/// <summary>
/// Defines the objects which compose the 'Dependencies tree', used in the build process.
/// </summary>

#include "pch.h"
#include "DependencyTree.h"
#include "compileError.h"
#include "utils.h"
#include "astSerialization.h"

//TODO: filesystem namespace is supossed to be already in 'std' namespace.
//Change as appropiate of put an '#ifdef' to handle different compilers / standard library versions.
namespace fs = std::experimental::filesystem;

using namespace std;

/// <summary>
/// Constructor. Initialiaces the module path, and the list of sources.
/// </summary>
/// <param name="modulePath"></param>
ModuleNode::ModuleNode(const std::string& modulePath)
    :m_path(modulePath)
{
    fs::path	modPath(modulePath);
    auto		extension = modPath.extension();

    if (extension == ".fast")
    {
        //It is an already compiled module
        if (!tryLoadAst(modPath.u8string()))
        {
            throw CompileError::create(
                ScriptPosition(),
                ETYPE_ERROR_LOADING_COMPILED_MODULE_1,
                modulePath.c_str()
            );
        }
    }
    else
    {
        //It is a source folder, create source objects.
        auto moduleObj = SourceModule::create(modulePath);
        auto sourcePaths = getModuleSources(modulePath);

        for (auto& srcFile : sourcePaths)
        {
            //TODO: Verify that this is a reliable way to get the file name.
            string name = srcFile.substr(modulePath.size()+1);
            auto fileObj = SourceFile::create(moduleObj, name);
            m_sources.emplace_back(new SourceFileNode(fileObj));
        }

        //Also try to load compiled module. But this time it does not throw an exception if
        //it fails.
        string	compPathStr = getCompiledPath();

        if (checkUpdated(compPathStr))
            tryLoadAst(compPathStr);
    }
}

/// <summary>
/// Adds a new module from which this one depends on.
/// </summary>
/// <param name="node"></param>
void ModuleNode::addDependency(ModuleNodePtr node)
{
    m_dependencies.emplace_back(move(node));
}

/// <summary>
/// Walks source nodes of this module.
/// </summary>
/// <param name="fn"></param>
void ModuleNode::walkSources(std::function<void(SourceFileNode*)> fn)const
{
    for (auto& src : m_sources)
        fn(src.get());
}

/// <summary>
/// Wals the nodes on which this module depends.
/// </summary>
/// <param name="fn"></param>
void ModuleNode::walkDependencies(std::function<void(ModuleNode*)> fn)const
{
    for (auto& dep : m_dependencies)
        fn(dep.get());
}

/// <summary>
/// GEts the name of this module.
/// </summary>
/// <returns></returns>
std::string ModuleNode::name()const
{
    return removeExt(fileFromPath(m_path));
}

/// <summary>
/// Sets the 'compiled AST node' field, and saves the AST to a file.
/// If it fails to save it, throws an exception, and the AST field is not set.
/// </summary>
/// <param name="ast"></param>
void ModuleNode::setAST(Ref<AstNode> ast)
{
    string path = getCompiledPath();

    try
    {
        createDirIfNotExist(parentPath(path));
        serializeAST(path, ast);
        m_compiledAst = ast;
    }
    catch (exception & ex)
    {
        throw CompileError::create(
            ScriptPosition(),
            ETYPE_WRITING_RESULT_FILE_2,
            path.c_str(),
            ex.what()
        );
    }
}


/// <summary>
/// Gets the path of the compiled file.
/// </summary>
/// <returns></returns>
std::string ModuleNode::getCompiledPath()const
{
    fs::path	modPath(m_path);

    if (modPath.extension() == ".fast")
        return m_path;
    else
    {
        fs::path    base(getBinDir());
        auto        compPath = base / (this->name() + ".fast");
        
        return compPath.u8string();
    }
}

/// <summary>
/// Gets the path where to store the generated 'C' source file.
/// </summary>
/// <returns></returns>
std::string ModuleNode::getCFilePath()const
{
    fs::path	base(getIntermediateDir());
    auto		result = base / (this->name() + ".c");

    return result.u8string();
}

/// <summary>
/// Gets the path where intermeadiate products of the compilation process
/// are stored.
/// </summary>
/// <returns></returns>
std::string ModuleNode::getIntermediateDir()const
{
    fs::path	modPath(m_path);
    auto		result = modPath / "int";

    return result.u8string();
}

/// <summary>
/// Gets the path where results (binaries) are stored for this module.
/// </summary>
/// <returns></returns>
std::string ModuleNode::getBinDir()const
{
    fs::path	modPath(m_path);
    auto		result = modPath / "bin";

    return result.u8string();
}


/// <summary>
/// Tries to load the AST from a file.
/// </summary>
/// <param name="path"></param>
/// <returns></returns>
bool ModuleNode::tryLoadAst(const std::string& path)
{
    try
    {
        m_compiledAst = deSerializeAST(path);
        //TODO: It should check AST integrity... A corrupted AST may crash the compiler.
        return true;
    }
    catch (exception &)
    {
        return false;
    }
}

/// <summary>
/// Checks if the compiled file is up-to-date regarding to the module sources.
/// </summary>
/// <remarks>It just take into account module sources, not its dependency modules.</remarks>
/// <param name="compPath"></param>
/// <returns></returns>
bool ModuleNode::checkUpdated(const std::string& compPath)
{
    error_code	ec;

    if (!fs::is_regular_file(fs::status(compPath)))
        return false;

    auto compTime = fs::last_write_time(compPath, ec);

    for (auto& srcFile : m_sources)
    {
        const string& srcPath = srcFile->path();

        if (!fs::is_regular_file(fs::status(srcPath)))
            return false;

        auto srcTime = fs::last_write_time(srcPath, ec);
        if (srcTime > compTime)
            return false;
    }

    return true;
}



/// <summary>
/// Gets the list of source files of a module, given its path.
/// It looks for them in the file system.
/// </summary>
/// <param name="modulePath"></param>
/// <returns></returns>
StrList ModuleNode::getModuleSources(const std::string& modulePath)
{
    StrList		result;

    for (auto entry : fs::directory_iterator(modulePath))
    {
        bool isSource = fs::is_regular_file(entry.status());

        isSource = isSource && entry.path().extension() == ".fil";
        isSource = isSource && entry.path().filename().c_str()[0] != '_';

        if (isSource)
            result.push_back(entry.path().u8string());
    }

    return result;
}
