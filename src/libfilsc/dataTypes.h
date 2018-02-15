/// <summary>
/// This file contains the classes which define the data types used
/// in FIL-S programs.
/// </summary>
#pragma once

#include "refCountObj.h"

/// <summary>
/// Describes the possible data types.
/// </summary>
enum EDataType
{
	DT_VOID,
	DT_INT,
	DT_BOOL,
	DT_TUPLE,
	DT_FUNCTION
};

/// <summary>
/// Base class for types.
/// </summary>
class BaseType : public RefCountObj
{
public:
	virtual ~BaseType() {}

	virtual EDataType type()const = 0;

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

	virtual EDataType type()const
	{
		return m_type;
	}

protected:
	DefaultType(EDataType type) : m_type(type) {}

private:
	EDataType m_type;
};

/// <summary>
/// Class for tuple data types
/// </summary>
class TupleType : public BaseType
{
public:

	virtual EDataType type()const
	{
		return DT_TUPLE;
	}

private:
	std::vector<Ref<BaseType>>	m_members;
	std::map<std::string, int>	m_names;
};

/// <summary>
/// Class for function types
/// </summary>
class FunctionType
{
public:

	virtual EDataType type()const
	{
		return DT_FUNCTION;
	}

private:
	Ref<TupleType>	m_parameters;
	Ref<BaseType>	m_returnType;
};