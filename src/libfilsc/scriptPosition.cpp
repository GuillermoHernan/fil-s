/// <summary>
/// Classes to handle source and object code positions.
/// </summary>

#include "pch.h"
#include "ScriptPosition.h"
#include "utils.h"

using namespace std;

/**
 * String representation of a Script position.
 * @return
 */
string ScriptPosition::toString()const
{
    string result = "[";

    result += to_string(line());
    result += ",";
    result += to_string(column());
    result += "]";

    if (m_file != nullptr)
    {
        result += "(";
        result += m_file->toString();
        result += ")";
    }

    return result;
}

/// <summary>
/// Creates a source module object in the heap, and returns a shared pointer to it.
/// </summary>
/// <param name="path"></param>
/// <returns></returns>
std::shared_ptr<SourceModule> SourceModule::create(const std::string& path)
{
    return shared_ptr<SourceModule>(new SourceModule(path));
}

/// <summary>
/// Creates a 'SourceFile' object in the heap, and returns a shared pointer to it.
/// </summary>
/// <param name="module"></param>
/// <param name="name"></param>
/// <returns></returns>
SourceFilePtr SourceFile::create(SourceModulePtr module, const std::string& name)
{
    return SourceFilePtr(new SourceFile(module, name));
}

/// <summary>
/// Gets the file path.
/// </summary>
/// <returns></returns>
std::string SourceFile::path()const
{
    if (m_module != nullptr)
        return joinPaths(m_module->path(), m_name);
    else
        return m_name;
}
