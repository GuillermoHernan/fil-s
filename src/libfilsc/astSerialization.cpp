/// <summary>
/// AstSerializeContext:
/// Stores information needed during AST tree serialization process.
/// For example, the list of datatypes.
/// </summary>
/// 
#include "pch.h"
#include "astSerialization.h"
#include "json11.hpp"
#include "utils.h"

using namespace std;

const int INDENT_SIZE = 2;

using json11::Json;

/// <summary>
/// Constructor. Just initializes 'm_output'.
/// </summary>
/// <param name="output"></param>
AstSerializeContext::AstSerializeContext(std::ostream& output)
    : m_output(output)
{
}

/// <summary>
/// Serializes a full AST tree.
/// </summary>
/// <param name="root"></param>
void AstSerializeContext::serializeAST(AstNode* root)
{
    auto ast = serializeNode(root);

    m_output << ast.dump();
}

/// <summary>
/// Returns a reference to a data type object
/// </summary>
/// <param name="node"></param>
/// <returns>Numeric reference as a hexadecimal string.</returns>
std::string AstSerializeContext::getNodeRef(const AstNode* node)
{
    if (node->getType() == AST_DEFAULT_TYPE)
        return node->getName();
    else if (astIsVoidType(node))
        return "";

    int value = 0;
    auto it = m_nodeIds.find(node);

    if (it == m_nodeIds.end())
    {
        value = m_nextId++;
        m_nodeIds[node] = value;
    }
    else
        value = it->second;

    std::stringstream stream;

    stream << std::hex << value;
    return stream.str();
}

/// <summary>
/// Serializes an AST node to JSON format.
/// </summary>
/// <param name="node"></param>
/// <returns></returns>
Json AstSerializeContext::serializeNode(const AstNode* node)
{
    Json::object		result;

    result["id"] = getNodeRef(node);
    result["type"] = astTypeToString(node->getType());
    result["name"] = node->getName();
    result["value"] = node->getValue();
    result["flags"] = to_string(node->getFlags());
    result["dataType"] = getNodeRef(node->getDataType());

    Json::array	children;

    for (auto child : node->children())
    {
        if (child.isNull())
            children.emplace_back(Json());
        else
            children.emplace_back(serializeNode(child.getPointer()));
    }

    result["children"] = move(children);

    return result;
}

/// <summary>
/// Registers the references of a node into the serialization context.
/// </summary>
/// <param name="node"></param>
/// <param name="id"></param>
/// <param name="dataTypeId"></param>
void AstDeserializeContext::registerNode(AstNode* node, const std::string& id, const std::string& dataTypeId)
{
    assert(node != nullptr);
    assert(m_node2TypeId.count(node) == 0);

    if (id.empty())
        throw exception("Corrupted AST file: node without id");

    if (m_id2Node.count(id) > 0)
    {
        string message = "Corrupted AST file: repeated node id: " + id;
        throw exception(message.c_str());
    }

    m_id2Node[id] = node;
    m_node2TypeId[node] = dataTypeId;
}

/// <summary>
/// Resolves a data type reference with the ids gathered wuring parsing.
/// </summary>
/// <param name="node"></param>
/// <returns></returns>
AstNode* AstDeserializeContext::getDataType(AstNode* node)
{
    assert(node != nullptr);
    assert(m_node2TypeId.count(node) == 1);

    string	typeId = m_node2TypeId[node];

    if (typeId.empty())
        return astGetVoid();
    else if (typeId == "int")
        return astGetInt();
    else if (typeId == "bool")
        return astGetBool();

    auto	it = m_id2Node.find(typeId);

    if (it != m_id2Node.end())
    {
        string message = "Corrupted AST file: unknown data type id: " + typeId;
        throw exception(message.c_str());
    }

    return it->second;
}

/// <summary>
/// Parses an AST node form its JSON representation
/// </summary>
/// <param name="jsNode"></param>
/// <param name="types"></param>
/// <returns></returns>
Ref<AstNode> parseAstNode(const Json& jsNode, AstDeserializeContext& ctx)
{
    string	id = jsNode["id"].string_value();
    auto	type = astTypeFromString(jsNode["type"].string_value());
    string	name = jsNode["name"].string_value();
    string	value = jsNode["value"].string_value();
    int		flags = jsNode["flags"].int_value();
    string	dataTypeRef = jsNode["dataType"].string_value();

    auto astNode = AstNode::create(type, ScriptPosition(), name, value, flags);

    ctx.registerNode(astNode.getPointer(), id, dataTypeRef);

    auto& childrenNodes = jsNode["children"].array_items();
    for (auto &childJs : childrenNodes)
    {
        Ref<AstNode>	childNode;

        if (!childJs.is_null())
            childNode = parseAstNode(childJs, ctx);

        astNode->addChild(childNode);
    }

    return astNode;
}

/// <summary>
/// Restores the data types to a just deserialized AST tree
/// </summary>
/// <param name="root"></param>
/// <param name="ctx"></param>
void restoreDataTypes(Ref<AstNode> root, AstDeserializeContext& ctx)
{
    auto	type = ctx.getDataType(root.getPointer());

    root->setDataType(type);

    for (auto child : root->children())
        restoreDataTypes(child, ctx);
}

/// /// <summary>
/// Parses the AST from its text-serialization format.
/// </summary>
/// <param name="text"></param>
/// <returns></returns>
Ref<AstNode> parseAST(const char* text)
{
    string	errText;
    auto	parsedJS = Json::parse(text, errText);

    if (parsedJS.is_null())
    {
        string message = "Error parsing AST: " + errText;
        throw exception(message.c_str());
    }

    AstDeserializeContext	ctx;

    auto root = parseAstNode(parsedJS, ctx);

    restoreDataTypes(root, ctx);

    return root;
}


/// <summary>
/// Writes an AST tree to a file.
/// </summary>
/// <param name="path"></param>
/// <param name="node"></param>
void serializeAST(const std::string& path, Ref<AstNode> node)
{
    ofstream	outFile;

    outFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    outFile.open(path);

    outFile << node;
}


/// <summary>
/// Reads an AST tree from a file.
/// </summary>
/// <param name="path"></param>
/// <returns></returns>
Ref<AstNode> deSerializeAST(const std::string& path)
{
    string content = readTextFile(path);

    if (content == "")
    {
        string message = string("Cannot read AST file: " + path);
        throw exception(message.c_str());
    }

    return parseAST(content.c_str());
}


/// <summary>
/// Writes an AST tree to a file.
/// </summary>
/// <param name="output"></param>
/// <param name="node"></param>
/// <returns></returns>
std::ostream& operator << (std::ostream& output, Ref<AstNode> node)
{
    AstSerializeContext	ctx(output);

    ctx.serializeAST(node.getPointer());
    return output;
}
