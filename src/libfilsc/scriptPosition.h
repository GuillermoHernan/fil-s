/// <summary>
/// Classes to handle source and object code positions.
/// </summary>

#pragma once

#include <string>
#include <memory>

class SourceFile;
typedef std::shared_ptr<SourceFile>	SourceFilePtr;

/// <summary>
///  Indicates a position inside a script file (line / column)
/// </summary>
class ScriptPosition
{
public:
    ScriptPosition() :
        m_line(-1), m_column(-1)
    {
    }

    ScriptPosition(SourceFilePtr file, int line, int col) :
        m_file(file), m_line(line), m_column(col)
    {
    }

    ScriptPosition(const ScriptPosition& refPos, int line, int col) :
        m_file(refPos.m_file), m_line(line), m_column(col)
    {
    }

    int				line()const { return m_line; }
    int				column()const { return m_column; }
    SourceFilePtr	file()const { return m_file; }

    std::string toString()const;

    bool operator < (const ScriptPosition& b)const
    {
        if (line() < b.line())
            return true;
        else if (line() == b.line())
            return column() < b.column();
        else
            return false;
    }

    bool operator == (const ScriptPosition& b)const
    {
        return (line() == b.line() && column() == b.column());
    }

    bool operator > (const ScriptPosition& b)const
    {
        return !(*this <= b);
    }

    bool operator <= (const ScriptPosition& b)const
    {
        return (*this < b || *this == b);
    }

    bool operator >= (const ScriptPosition& b)const
    {
        return !(*this < b);
    }
    bool operator != (const ScriptPosition& b)const
    {
        return !(*this == b);
    }

private:
    int				m_line;
    int				m_column;
    SourceFilePtr	m_file;
};//struct ScriptPosition

/// <summary>
/// Identification data of a source module.
/// </summary>
class SourceModule
{
public:
    static std::shared_ptr<SourceModule>	create(const std::string& path);

    SourceModule(const std::string& path)
        : m_path(path)
    {}

    const std::string& path()const
    {
        return m_path;
    }

private:
    std::string m_path;
};
typedef std::shared_ptr<SourceModule> SourceModulePtr;

/// <summary>
/// Identifies a source code file.
/// </summary>
class SourceFile
{
public:
    SourceFile(SourceModulePtr module, const std::string& name)
        :m_module(module), m_name(name)
    {}

    static SourceFilePtr create(SourceModulePtr module, const std::string& name);

    std::string toString()const
    {
        return path();
    }
    std::string path()const;

private:
    SourceModulePtr m_module;
    std::string		m_name;
};