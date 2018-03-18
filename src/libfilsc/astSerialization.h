#pragma once

#include "ast.h"

void serializeAST(const std::string& path, Ref<AstNode> node);
Ref<AstNode> deSerializeAST(const std::string& path);

std::ostream& operator << (std::ostream& output, Ref<AstNode> node);

Ref<AstNode>	parseAST(const char* text);

namespace json11
{
    class Json;
}

/// <summary>
/// Stores information needed during AST tree serialization process.
/// </summary>
class AstSerializeContext
{
public:
    AstSerializeContext(std::ostream& output);

    void			serializeAST(AstNode* root);
    std::string		getNodeRef(const AstNode* node);

private:
    json11::Json	serializeNode(const AstNode* root);

private:
    std::ostream&	m_output;

    std::map <const AstNode*, int>	m_nodeIds;
    int								m_nextId = 1;
};

/// <summary>
/// Stores information needed during AST tree deserialization process.
/// For example, the list of datatypes.
/// </summary>
class AstDeserializeContext
{
public:
    void		registerNode(AstNode* node, const std::string& id, const std::string& dataTypeId);
    AstNode*	getDataType(AstNode* node);

private:
    std::map<std::string, AstNode*>	m_id2Node;
    std::map<AstNode*, std::string>	m_node2TypeId;
};
