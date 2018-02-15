#include "pch.h"
#include "dataTypes.h"

using namespace std;

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
/// Creates a 'function' data type from an AST node.
/// </summary>
/// <param name="node"></param>
/// <returns></returns>
Ref<FunctionType> FunctionType::create(Ref<AstNode> node)
{
	assert(node->getType() == AST_FUNCTION);

	auto fnType = refFromNew(new FunctionType(node->getName()));

	assert(node->child(0)->getDataType()->type() == DT_TUPLE);
	fnType->m_parameters = node->child(0)->getDataType().staticCast<TupleType>();

	if (node->childExists(1))
		fnType->m_returnType = node->child(1)->getDataType();

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
