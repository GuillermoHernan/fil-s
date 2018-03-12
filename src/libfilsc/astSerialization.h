#pragma once

#include "ast.h"

class BaseType;

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
/// For example, the list of datatypes.
/// </summary>
class AstSerializeContext
{
public:
	AstSerializeContext(std::ostream& output);

	void			serializeAST(AstNode* root);
	std::string		datatypeRef(const BaseType* type);

private:
	int generateIds(AstNode* root, int nextId = 1);

	json11::Json	serializeTypes();
	json11::Json	serializeNode(const AstNode* root);
	json11::Json	serializeType(const BaseType* type);

private:
	int generateIds(const BaseType* type, int nextId);

private:
	std::ostream&	m_output;

	std::map <const BaseType*, int>	m_typeIds;
	std::map <const AstNode*, int>	m_nodeIds;
};
