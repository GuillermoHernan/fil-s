#include "pch.h"
#include "dataTypes.h"
#include "astSerialization.h"
#include "utils.h"

using namespace std;

/// <summary>
/// Gets the string represention of a 'EDataType'
/// </summary>
/// <param name="type"></param>
/// <returns></returns>
std::string toString(EDataType type)
{
	const char* types[] = {
		"DT_VOID",
		"DT_INT",
		"DT_BOOL",
		"DT_TUPLE",
		"DT_FUNCTION",
		"DT_ACTOR",
		"DT_INPUT",
		"DT_OUTPUT",
	};
	static_assert (arrCount(types) == DT_COUNT, "'toString(EDataType)' function is not updated!");
	assert(type >= 0 && type < DT_COUNT);

	return types[type];
}

/// <summary>
/// Transforms the string representation of a 'EDataType' into a 'EDataType' value.
/// Throw an exception if the string does not match any enum value.
/// </summary>
/// <param name="str"></param>
/// <returns></returns>
EDataType eDataTypeFromString(const std::string& str)
{
	static map<string, EDataType>	types;

	if (types.empty())
	{
		types["DT_VOID"] = DT_VOID;
		types["DT_INT"] = DT_INT;
		types["DT_BOOL"] = DT_BOOL;
		types["DT_TUPLE"] = DT_TUPLE;
		types["DT_FUNCTION"] = DT_FUNCTION;
		types["DT_ACTOR"] = DT_ACTOR;
		types["DT_INPUT"] = DT_INPUT;
		types["DT_OUTPUT"] = DT_OUTPUT;

		assert(types.size() == DT_COUNT);
	}

	auto it = types.find(trim(str));

	if (it != types.end())
		return it->second;
	else
	{
		string message = "'" + str + "' is not a data type type.";
		throw exception(message.c_str());
	}
}

/// <summary>
/// Generic data type constructor (factory) function.
/// Selects the appropriate data type subclass based on 'type' parameter.
/// </summary>
/// <param name="type"></param>
/// <param name="name"></param>
/// <param name="params"></param>
/// <param name="retType"></param>
/// <returns></returns>
Ref<BaseType> BaseType::create(
	EDataType type,
	const std::string& name,
	Ref<TupleType> params,
	Ref<BaseType> retType
)
{
	switch (type)
	{
	case DT_VOID:
		return DefaultType::createVoid();

	case DT_INT:
		return DefaultType::createInt();
	case DT_BOOL:
		return DefaultType::createBool();

	case DT_TUPLE:
		return TupleType::create(name);

	case DT_FUNCTION:
		return FunctionType::create(name, params, retType);

	case DT_ACTOR:
		return ActorType::create(name, params);

	case DT_INPUT:
	case DT_OUTPUT:
		return MessageType::create(type, name, params);

	default:
		assert(!"Invalid 'type' parameter");
		return DefaultType::createVoid();
	}
}

Ref<TupleType> BaseType::getParameters()const
{
	static const auto emptyTuple = TupleType::create();
	return emptyTuple;
}

Ref<BaseType> BaseType::getReturnType()const
{
	return DefaultType::createVoid();
}

/// <summary>
/// Gets the list of types on which this type depends on.
/// </summary>
/// <param name="recursive">Set to true to recursively scan for the comprehensive list 
/// of dependencies. If false, just gets the direct dependencies</param>
/// <returns></returns>
std::vector<const BaseType*> BaseType::getDependencies(bool recursive)const
{
	set<const BaseType*>	deps;
	getDependencies(deps, recursive, false);

	return vector<const BaseType*>(deps.begin(), deps.end());
}

/// <summary>
/// Gets the list of types on which this type depends on.
/// </summary>
/// <param name="typeSet">Set of types where dependencies are stored.</param>
/// <param name="recursive">Set to true to recursively scan for the comprehensive list 
/// of dependencies. If false, just gets the direct dependencies</param>
/// <param name="addSelf">Set to 'true' to add this type to the list.</param>
void BaseType::getDependencies(std::set<const BaseType*>& typeSet, bool recursive, bool addSelf)const
{
	if (typeSet.count(this) > 0)
		return;
	else if (addSelf)
	{
		typeSet.insert(this);
		if (!recursive)
			return;
	}

	auto params = getParameters();
	auto retType = getReturnType();

	if (params.notNull())
		params->getDependencies(typeSet, recursive, true);

	if (retType.notNull())
		retType->getDependencies(typeSet, recursive, true);
}


/// <summary>
/// Creates an integer default type
/// </summary>
Ref<DefaultType> DefaultType::createInt()
{
	static Ref<DefaultType> instance = refFromNew(new DefaultType("int", DT_INT));
	return instance;
}

/// <summary>
/// Creates a boolean default type
/// </summary>
Ref<DefaultType> DefaultType::createBool()
{
	static Ref<DefaultType> instance = refFromNew(new DefaultType("bool", DT_BOOL));
	return instance;
}

/// <summary>
/// Creates a void default type
/// </summary>
Ref<DefaultType> DefaultType::createVoid()
{
	static Ref<DefaultType> instance = refFromNew(new DefaultType("", DT_VOID));
	return instance;
}

/// <summary>
/// 'toString' function is overloaded to print void type as an empty tuple.
/// </summary>
std::string	DefaultType::toString()const
{
	if (m_type == DT_VOID)
		return "()";
	else
		return BaseType::toString();
}


/// <summary>
/// Creates a tuple datatype
/// </summary>
/// <returns></returns>
Ref<TupleType> TupleType::create(const std::string& name)
{
	return refFromNew(new TupleType(name));
}

/// <summary>
/// Gets the type enumeration of a tuple. It may be voi if it has no members.
/// </summary>
/// <returns></returns>
EDataType TupleType::type()const
{
	if (m_members.empty())
		return DT_VOID;
	else
		return DT_TUPLE;
}

/// <summary>
/// Tuple type to string.
/// </summary>
/// <returns></returns>
std::string	TupleType::toString()const
{
	ostringstream	output;

	output << "(";
	const int count = memberCount();

	for (int i = 0; i < count; ++i)
	{
		if (i > 0)
			output << ",";

		output << getMemberType(i)->toString();
	}
	output << ")";

	return output.str();
}

/// <summary>Looks for a member by name.</summary>
/// <param name="name"></param>
/// <returns>The index of the member or -1 if not found</returns>
int TupleType::findMemberByName(const std::string& name)const
{
	auto it = m_names.find(name);

	if (it != m_names.end())
		return it->second;
	else
		return -1;
}

Ref<BaseType> TupleType::getMemberType(int index)const
{
	return m_members[index].type;
}

const string& TupleType::getMemberName(int index)const
{
	return m_members[index].name;
}

void TupleType::addMember(Ref<BaseType> type, const std::string& name)
{
	int index = (int)m_members.size();
	m_members.push_back(MemberInfo{type, name});

	if (name != "")
		m_names[name] = index;
}

/// <summary>
/// Walks all tuple members, calling the supplied function for each one.
/// </summary>
/// <param name="nodeFn"></param>
void TupleType::walkMembers(std::function<void(BaseType*, const std::string&)> nodeFn)const
{
	for (auto node : m_members)
		nodeFn(node.type.getPointer(), node.name);
}

/// <summary>
/// Gets the depdendences of the tuple.
/// </summary>
/// <param name="typeSet"></param>
/// <param name="recursive"></param>
/// <param name="addSelf"></param>
void TupleType::getDependencies(
	std::set<const BaseType*>& typeSet,
	bool recursive,
	bool addSelf
)const
{
	BaseType::getDependencies(typeSet, recursive, addSelf);

	if (!addSelf || recursive)
	{
		for (int i = 0; i < memberCount(); ++i)
			getMemberType(i)->getDependencies(typeSet, recursive, true);
	}
}

/// <summary>
/// Creates a 'function' data type from an AST node.
/// </summary>
/// <param name="node"></param>
/// <returns></returns>
Ref<FunctionType> FunctionType::create(Ref<AstNode> node)
{
	assert(node->getType() == AST_FUNCTION);

	auto fnType = refFromNew(new FunctionType(node->getName()));
	auto paramsType = node->child(0)->getDataType().dynamicCast<TupleType>();

	assert(paramsType.notNull());
	fnType->m_parameters = paramsType;

	if (node->childExists(1))
		fnType->m_returnType = node->child(1)->getDataType();
	else
		fnType->m_returnType = DefaultType::createVoid();

	return fnType;
}

/// <summary>
/// Creates a 'function' data type.
/// </summary>
/// <param name="name"></param>
/// <param name="params"></param>
/// <param name="retType"></param>
/// <returns></returns>
Ref<FunctionType> FunctionType::create(
	const std::string& name,
	Ref<TupleType> params,
	Ref<BaseType> retType
)
{
	auto result = refFromNew(new FunctionType(name));

	result->m_parameters = params;
	result->setReturnType(retType);

	return result;
}


/// <summary>
/// String representation of a 'FunctionType'
/// </summary>
/// <returns></returns>
std::string	FunctionType::toString()const
{
	string result = "function" + m_parameters->toString();

	if (m_returnType.notNull())
		result += ":" + m_returnType->toString();

	return result;
}

/// <summary>
/// Creates an actor data type from the AST node where it is defined.
/// </summary>
/// <param name="node"></param>
/// <returns></returns>
Ref<ActorType> ActorType::create(Ref<AstNode> node)
{
	assert(node->getType() == AST_ACTOR);
	assert(node->childCount() > 0);
	assert(node->child(0)->getType() == AST_TUPLE_DEF);

	auto result = refFromNew(new ActorType(node->getName()));

	result->m_parameters = node->child(0)->getDataType().staticCast<TupleType>();

	//Parameters are also members
	result->walkMembers([result](auto type, auto name) {
		result->addMember(type, name);
	});

	//Add the members defined in the body.
	for (size_t i = 1; i < node->childCount(); ++i)
	{
		assert(node->childExists(i));
		auto child = node->child(i);

		result->addMember(child->getDataType(), child->getName());
	}

	return result;
}

/// <summary>
/// Creates an actor data type
/// </summary>
/// <param name="name"></param>
/// <param name="params"></param>
/// <returns></returns>
Ref<ActorType> ActorType::create(const std::string& name, Ref<TupleType> params)
{
	auto result = refFromNew(new ActorType(name));

	result->m_parameters = params;
	return result;
}

/// <summary>
/// Returns an string representation of the actor type.
/// </summary>
/// <returns></returns>
string ActorType::toString()const
{
	return string("Actor '") + getName() + "'";
}

/// <summary>
/// Creates a 'message' data type from the node in which it is defined.
/// </summary>
/// <param name="node"></param>
/// <returns></returns>
Ref<MessageType> MessageType::create(Ref<AstNode> node)
{
	assert(node->getType() == AST_INPUT || node->getType() == AST_OUTPUT);
	assert(node->childCount() > 0);
	assert(node->child(0)->getType() == AST_TUPLE_DEF);

	EDataType	type = node->getType() == AST_INPUT ? DT_INPUT : DT_OUTPUT;
	auto		result = refFromNew(new MessageType(node->getName(), type));

	result->m_parameters = node->child(0)->getDataType().staticCast<TupleType>();
	return result;
}

/// <summary>
/// Creates a 'message' (input or output) data type.
/// </summary>
/// <param name="type"></param>
/// <param name="name"></param>
/// <param name="params"></param>
/// <returns></returns>
Ref<MessageType> MessageType::create(
	EDataType type,
	const std::string& name,
	Ref<TupleType> params
)
{
	assert(type == AST_INPUT || type == AST_OUTPUT);

	auto result = refFromNew(new MessageType(name, type));
	result->m_parameters = params;

	return result;
}


/// <summary>
/// String representation of a 'MessageType'
/// </summary>
/// <returns></returns>
std::string	MessageType::toString()const
{
	string header = m_type == DT_INPUT ? "input" : "output";

	return header + m_parameters->toString();
}
