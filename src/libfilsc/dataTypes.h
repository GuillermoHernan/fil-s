/// <summary>
/// This file contains the classes which define the data types used
/// in FIL-S programs.
/// </summary>
#pragma once

#include "refCountObj.h"
#include "ast.h"

/// <summary>
/// Describes the possible data types.
/// </summary>
enum EDataType
{
	DT_VOID,
	DT_INT,
	DT_BOOL,
	DT_TUPLE,
	DT_FUNCTION,
	DT_ACTOR,
	DT_INPUT,
	DT_OUTPUT
};

class TupleType;

/// <summary>
/// Base class for types.
/// </summary>
class BaseType : public RefCountObj
{
public:
	virtual ~BaseType() {}

	virtual EDataType	type()const=0;
	virtual std::string	toString()const
	{
		return m_name;
	}

	const std::string& getName()const
	{
		return m_name;
	}

	virtual bool canBeCalled()const
	{
		return false;
	}
	virtual Ref<TupleType> getParameters()const;
	virtual Ref<BaseType> getReturnType()const;

protected:
	BaseType(const std::string & name) : m_name(name) {}
	BaseType(){}

private:
	const std::string	m_name;
};

/// <summary>
/// Class for built-in types (bool, int, etc)
/// </summary>
class DefaultType : public BaseType
{
public:
	static Ref<DefaultType>	createInt();
	static Ref<DefaultType>	createBool();
	static Ref<DefaultType>	createVoid();

	virtual EDataType type()const override
	{
		return m_type;
	}

	virtual std::string	toString()const override;

protected:
	DefaultType(const std::string& name, EDataType type) : BaseType (name), m_type(type) {}

private:
	EDataType m_type;
};

/// <summary>
/// Class for tuple data types
/// </summary>
class TupleType : public BaseType
{
public:
	static Ref<TupleType> create(const std::string& name = "");

	virtual EDataType	type()const override;
	virtual std::string	toString()const override;

	int memberCount()const
	{
		return (int)m_members.size();
	}

	int				findMemberByName(const std::string& name)const;

	Ref<AstNode>	getMemberNode(int index)const;
	Ref<BaseType>	getMemberType(int index)const;

	void			addMember(Ref<AstNode> node);

	void			walkMembers(std::function<void(Ref<AstNode>)> nodeFn);

protected:
	TupleType(const std::string& name):BaseType(name) {}

private:
	std::vector<Ref<AstNode>>	m_members;
	std::map<std::string, int>	m_names;
};

/// <summary>
/// Class for function types
/// </summary>
class FunctionType : public BaseType
{
public:
	static Ref<FunctionType> create(Ref<AstNode> node);

	virtual EDataType type()const override
	{
		return DT_FUNCTION;
	}

	virtual std::string	toString()const override;

	virtual Ref<TupleType> getParameters()const override
	{
		return m_parameters;
	}

	virtual Ref<BaseType> getReturnType()const override
	{
		return m_returnType;
	}

	virtual bool canBeCalled()const override
	{
		return true;
	}

	void setReturnType(Ref<BaseType> type)
	{
		m_returnType = type;
	}

protected:
	FunctionType(const std::string& name) : BaseType(name) {}

private:
	Ref<TupleType>	m_parameters;
	Ref<BaseType>	m_returnType;
};

/// <summary>
/// Data type for actors.
/// </summary>
class ActorType : public TupleType
{
public:
	static Ref<ActorType> create(Ref<AstNode> node);

	virtual EDataType type()const override
	{
		return DT_ACTOR;
	}

	virtual std::string	toString()const override;

	virtual Ref<TupleType> getParameters()const override
	{
		return m_parameters;
	}

protected:
	ActorType(const std::string& name) : TupleType(name) {}

private:
	Ref<TupleType>	m_parameters;

};

/// <summary>
/// Data type class for actor messages (input & output)
/// </summary>
class MessageType : public BaseType
{
public:
	static Ref<MessageType> create(Ref<AstNode> node);

	virtual EDataType type()const override
	{
		return m_type;
	}

	virtual std::string	toString()const override;

	virtual Ref<TupleType> getParameters()const override
	{
		return m_parameters;
	}

	virtual bool canBeCalled()const override
	{
		return true;
	}

protected:
	MessageType(const std::string& name, EDataType type) : BaseType(name), m_type(type) {}

private:
	EDataType		m_type;
	Ref<TupleType>	m_parameters;
};
