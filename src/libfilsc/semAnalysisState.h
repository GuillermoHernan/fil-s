#pragma once

#include "ast.h"
#include "SymbolScope.h"

#include <functional>

/// <summary>
/// Holds the current state of the semantic analyzer.
/// Keeps it between passes
/// </summary>
class SemAnalysisState
{
public:
    const Ref<SymbolScope>					rootScope;
    std::map < std::string, Ref<AstNode>>	modules;
    std::string								currentFile;

    SemAnalysisState();

    Ref<AstNode>		parent(unsigned index = 0)const;

    void				pushParent(Ref<AstNode> node);
    Ref<AstNode>		popParent();

    Ref<AstNode>		findParent(std::function<bool(Ref<AstNode>)> pred)const;

    Ref<SymbolScope>	getScope(const AstNode* node)const;
    Ref<SymbolScope>	getScope(Ref<AstNode> node)const;

    void				setScope(Ref<AstNode> node, Ref<SymbolScope> scope);

    Ref<AstNode>        registerUnnamedType(Ref<AstNode> tupleType);

    AstNodeList         getUnnamedTypes()const;

private:
    //To compare tuples in unnamed type registry.
    struct CompareTupleTypes {
        bool operator()(Ref<AstNode> typeA, Ref<AstNode> typeB);
    };

    std::vector<Ref<AstNode>> m_parents;
    std::map < const AstNode*, Ref<SymbolScope>> m_scopesMap;
    std::set < Ref<AstNode>, CompareTupleTypes> m_unnamedTypes;
};
