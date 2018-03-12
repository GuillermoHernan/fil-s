/// <summary>
/// AstSerializeContext:
/// Stores information needed during AST tree serialization process.
/// For example, the list of datatypes.
/// </summary>
/// 
#include "pch.h"
#include "astSerialization.h"
#include "dataTypes.h"
#include "lexer.h"
#include "json11.hpp"
#include "utils.h"
#include "dependencySolver.h"

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
	generateIds(root);

	auto types = serializeTypes();
	auto ast = serializeNode(root);

	Json result = Json::object{
		{ "types", types },
		{ "ast", ast },
	};

	m_output << result.dump();	
}


/// <summary>
/// Assigns a numeric ID to each AST node and data type found in the tree.
/// </summary>
/// <param name="root"></param>
int AstSerializeContext::generateIds(AstNode* root, int nextId)
{
	if (root == nullptr)
		return nextId;

	if (m_nodeIds.count(root) == 0)
		m_nodeIds[root] = nextId++;

	nextId = generateIds(root->getDataType().getPointer(), nextId);

	for (auto child : root->children())
		nextId = generateIds(child.getPointer(), nextId);

	return nextId;
}

/// <summary>
/// Generates id for a type.
/// </summary>
/// <param name="type"></param>
/// <param name="nextId"></param>
/// <returns></returns>
int AstSerializeContext::generateIds(BaseType* type, int nextId)
{
	if (m_typeIds.count(type) > 0)
		return nextId;

	m_typeIds[type] = nextId++;

	auto dependencies = type->getDependencies(true);

	for (auto t : dependencies)
		nextId = generateIds(t, nextId);

	return nextId;
}

/// <summary>
/// Returns a reference to a data type object
/// </summary>
/// <param name="type"></param>
/// <returns>Numeric reference as a hexadecimal string.</returns>
std::string AstSerializeContext::datatypeRef(const BaseType* type)
{
	int value = 0;
	auto it = m_typeIds.find(const_cast<BaseType*>(type));

	if (it == m_typeIds.end())
		return "";
	else
	{
		std::stringstream stream;

		stream << std::hex << it->second;
		return stream.str();
	}
}

/// <summary>Serializes all data types know by this context</summary>
/// <remarks>'generateIds' must have been called first, in order to collect
/// AST nodes and data types.</remarks>
Json AstSerializeContext::serializeTypes()
{
	Json::object		result;
	vector <BaseType*>	types;

	for (auto& dtEntry : m_typeIds)
		types.push_back(dtEntry.first);

	types = dependencySort<BaseType*>(types, [](auto type){
		return type->getDependencies(false);
	});

	for (auto& type : types)
		result[to_string(m_typeIds[type])] = serializeType(type);

	return result;
}

/// <summary>
/// Serializes an AST node to JSON format.
/// </summary>
/// <param name="node"></param>
/// <returns></returns>
Json AstSerializeContext::serializeNode(const AstNode* node)
{
	Json::object		result;

	result["type"] = astTypeToString(node->getType());
	result["name"] = node->getName();
	result["value"] = node->getValue();
	result["flags"] = to_string(node->getFlags());
	result["dataType"] = datatypeRef(node->getDataType().getPointer());

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
/// Serializes a data type.
/// </summary>
/// <param name="type"></param>
/// <returns></returns>
Json AstSerializeContext::serializeType(BaseType* type)
{
	Json::object		result;

	result["type"] = toString(type->type());
	result["name"] = type->getName();

	if (type->getParameters()->memberCount() > 0)
		result["params"] = datatypeRef(type->getParameters().getPointer());

	if (type->getReturnType()->type() != DT_VOID)
		result["return"] = datatypeRef(type->getReturnType().getPointer());

	auto	tuple = dynamic_cast<const TupleType*>(type);

	if (tuple != nullptr)
	{
		Json::array		members;

		tuple->walkMembers([&members, this](auto type, auto name) {
			auto member = Json::object{
				{ "name", name },
				{ "dataType", datatypeRef(type) }
			};
			members.emplace_back(member);
		});

		result["members"] = move(members);
	}

	return result;
}

//Maps type identifier (in serialized file) to data type objects
typedef map < string, Ref<BaseType>>	Id2TypeMap;
Ref<BaseType> parseType(const Json& typeNode, const Id2TypeMap& typeMap);

/// <summary>
/// Resolves a type reference in a serialized AST.
/// </summary>
/// <param name="typeId"></param>
/// <param name="typeMap"></param>
/// <returns></returns>
Ref<BaseType> resolveTypeRef(const std::string& typeId, const Id2TypeMap& typeMap)
{
	auto it = typeMap.find(typeId);

	if (it == typeMap.end())
		throw exception("AST file corrupted: cannot solve type reference.");
	else
		return it->second;
}

/// <summary>
/// Parses the tuple members in the AST serialization format.
/// </summary>
/// <param name="membersNode"></param>
/// <param name="tuple"></param>
/// <param name="typeMap"></param>
void parseTupleMembers(const Json& membersNode, TupleType* tuple, const Id2TypeMap& typeMap)
{
	for (auto& memberNode : membersNode.array_items())
	{
		string name = memberNode["name"].string_value();
		string typeRef = memberNode["dataType"].string_value();
		auto type = resolveTypeRef(typeRef, typeMap);

		tuple->addMember(type, name);
	}
}

/// <summary>
/// Parses a data type from a JSON node.
/// </summary>
/// <param name="typeNode"></param>
/// <param name="typeMap">Necessary to resolve type references</param>
/// <returns></returns>
Ref<BaseType> parseType(const Json& typeNode, const Id2TypeMap& typeMap)
{
	EDataType		type = eDataTypeFromString(typeNode["type"].string_value());
	string			name = typeNode["name"].string_value();
	Ref<BaseType>	params;
	Ref<BaseType>	retType;

	auto&	items = typeNode.object_items();
	auto	it = items.find("params");
	if (it != items.end())
		params = resolveTypeRef(it->second.string_value(), typeMap);

	it = items.find("return");
	if (it != items.end())
		retType = resolveTypeRef(it->second.string_value(), typeMap);

	auto	result = BaseType::create(type, name, params.dynamicCast<TupleType>(), retType);
	auto	tuple = dynamic_cast<TupleType*>(result.getPointer());

	if (tuple != nullptr)
		parseTupleMembers(typeNode["children"], tuple, typeMap);

	return result;
}

/// <summary>
/// Parses data types JSON node, and returns the parsed types.
/// </summary>
/// <param name="typesNode"></param>
/// <returns></returns>
Id2TypeMap parseTypes(const Json& typesNode)
{
	Id2TypeMap	result;

	for (auto& typeEntry : typesNode.object_items())
		result[typeEntry.first] = parseType(typeEntry.second, result);

	return result;
}

/// <summary>
/// Parses an AST node form its JSON representation
/// </summary>
/// <param name="jsNode"></param>
/// <param name="types"></param>
/// <returns></returns>
Ref<AstNode> parseAstNode(const Json& jsNode, const Id2TypeMap& types)
{
	auto	type = astTypeFromString(jsNode["type"].string_value());
	string	name = jsNode["name"].string_value();
	string	value = jsNode["value"].string_value();
	int		flags = jsNode["flags"].int_value();
	string	dataTypeRef = jsNode["dataType"].string_value();
	auto	dataType = resolveTypeRef(dataTypeRef, types);

	auto astNode = AstNode::create(type, ScriptPosition(), name, value, flags);

	astNode->setDataType(dataType);

	auto& childrenNodes = jsNode["children"].array_items();
	for (auto &childJs : childrenNodes)
	{
		Ref<AstNode>	childNode;

		if (!childJs.is_null())
			childNode = parseAstNode(childJs, types);

		astNode->addChild(childNode);
	}

	return astNode;
}

/// <summary>
/// Parses the AST from its text-serialization format.
/// </summary>
/// <param name="text"></param>
/// <returns></returns>
Ref<AstNode> parseAST (const char* text)
{
	string	errText;
	auto	parsed = Json::parse(text, errText);

	if (parsed.is_null())
	{
		string message = "Error parsing AST: " + errText;
		throw exception(message.c_str());
	}

	auto types = parseTypes(parsed["types"]);
	return parseAstNode(parsed["ast"], types);
}


/// <summary>
/// Writes an AST tree to a file.
/// </summary>
/// <param name="path"></param>
/// <param name="node"></param>
void serializeAST(const std::string& path, Ref<AstNode> node)
{
	ofstream	outFile(path);

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
