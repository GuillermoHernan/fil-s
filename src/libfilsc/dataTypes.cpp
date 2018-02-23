#include "pch.h"
#include "dataTypes.h"

using namespace std;

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

Ref<AstNode> TupleType::getMemberNode(int index)const
{
	return m_members[index];
}

Ref<BaseType> TupleType::getMemberType(int index)const
{
	return getMemberNode(index)->getDataType();
}

void TupleType::addMember(Ref<AstNode> node)
{
	int index = (int)m_members.size();

	m_members.push_back(node);
	string name = node->getName();

	if (name != "")
		m_names[name] = index;
}

/// <summary>
/// WAlks all tuple members, calling the supplied function for each one.
/// </summary>
/// <param name="nodeFn"></param>
void TupleType::walkMembers(std::function<void(Ref<AstNode>)> nodeFn)
{
	for (auto node : m_members)
		nodeFn(node);
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
/// Creates an actor data type from the AST node qhere it is defined.
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
	result->walkMembers([result](auto node) {
		result->addMember(node);
	});

	//Add the members defined in the body.
	for (size_t i = 1; i < node->childCount(); ++i)
	{
		assert(node->childExists(i));
		result->addMember(node->child(i));
	}

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
/// String representation of a 'MessageType'
/// </summary>
/// <returns></returns>
std::string	MessageType::toString()const
{
	string header = m_type == DT_INPUT ? "input" : "output";

	return header + m_parameters->toString();
}
