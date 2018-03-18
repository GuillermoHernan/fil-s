/// <summary>
/// Defines the objects which compose the 'Dependencies tree', used in the build process.
/// </summary>
#pragma once

#include <functional>
#include "ast.h"

class ModuleNode;
class SourceFileNode;
typedef std::unique_ptr<ModuleNode>	DepencencyTreePtr;
typedef std::unique_ptr<SourceFileNode>	SourceFileNodePtr;

typedef std::vector<std::string>	StrList;

/// <summary>
/// Branch node of the dependecy tree, which is a module.
/// </summary>
class ModuleNode
{
public:
    ModuleNode(const std::string& modulePath, const StrList& sourcePaths);

    void addDependency(DepencencyTreePtr node);

    bool buildNeeded()const
    {
        return m_compiledAst.isNull();
    }

    void walkSources(std::function<void(SourceFileNode*)> fn)const;
    void walkDependencies(std::function<void(ModuleNode*)> fn)const;

    std::string		name()const;

    Ref<AstNode> getAST()const
    {
        return m_compiledAst;
    }
    void setAST(Ref<AstNode> ast);

    //Paths
    const std::string&	path()const { return m_path; }
    std::string			getCompiledPath()const;
    std::string			getCFilePath()const;

private:
    bool tryLoadAst(const std::string& path);
    bool checkUpdated(const std::string& compPath);

private:
    std::string						m_path;
    std::vector<SourceFileNodePtr>	m_sources;
    std::vector<DepencencyTreePtr>	m_dependencies;

    Ref<AstNode>					m_compiledAst;
};

/// <summary>
/// A source file inside the dependency tree
/// </summary>
class SourceFileNode
{
public:
    SourceFileNode(SourceFilePtr ref)
        :m_ref(ref)
    {}

    std::string path()const
    {
        return m_ref->path();
    }

    SourceFilePtr ref()const
    {
        return m_ref;
    }

    Ref<AstNode> getAST()const
    {
        return m_ast;
    }
    void setAST(Ref<AstNode> ast)
    {
        m_ast = ast;
    }

private:
    SourceFilePtr	m_ref;
    Ref<AstNode>	m_ast;
};
