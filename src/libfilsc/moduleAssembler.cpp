/// <summary>
/// Functions to join the ASTs from several source files into a single AST.
///
/// Part of the build system.
/// </summary>

#include "pch.h"
#include "moduleAssembler.h"

bool isExportable(Ref<AstNode> node);

/// <summary>
/// Assembles a module node from a set of 'AST_SCRIPT' nodes.
/// </summary>
/// <param name="moduleName">Name of the module.</param>
/// <param name="sources">List of AST_SCRIPT nodes.</param>
/// <returns>Generated module node or a list of errors.</returns>
AssemblyResult assembleModule(const std::string& moduleName, const AstNodeList& sources)
{
    auto moduleNode = astCreateModule(moduleName);

    for (auto& script : sources)
    {
        assert(script->getType() == AST_SCRIPT);
        moduleNode->addChild(script);
    }

    //Symbol export
    for (auto& script : sources)
    {
        for (auto item : script->children())
        {
            if (isExportable(item))
                moduleNode->addChild(item);
        }
    }

    return moduleNode;
}

/// <summary>
/// Assigns imported modules to 'import' nodes.
/// </summary>
/// <param name="moduleNode"></param>
/// <param name="modules"></param>
/// <returns></returns>
AssemblyResult assignImportedModules(Ref<AstNode> moduleNode, const AstStr2NodesMap& modules)
{
    for (auto script : moduleNode->children())
    {
        if (script->getType() != AST_SCRIPT)
            break;

        for (auto item : script->children())
        {
            if (item->getType() == AST_IMPORT && !item->hasFlag(ASTF_EXTERN_C))
            {
                auto it = modules.find(item->getValue());

                assert(it != modules.end());
                item->setReference(it->second.getPointer());
            }
        }
    }//for

    return moduleNode;
}

/// <summary>
/// Checks if an AST node should be exported to be available in the whole module.
/// </summary>
/// <param name="node"></param>
/// <returns></returns>
bool isExportable(Ref<AstNode> node)
{
    if (node.isNull())
        return false;

    if (node->getName().empty())
        return false;

    if (node->getName()[0] == '_')
        return false;

    switch (node->getType())
    {
    case AST_TYPEDEF:
    case AST_DECLARATION:
    case AST_TUPLE_DEF:
    case AST_FUNCTION:
    case AST_FUNCTION_TYPE:
    case AST_ACTOR:
    case AST_MESSAGE_TYPE:
        return true;

    default:
        return false;
    }
}
