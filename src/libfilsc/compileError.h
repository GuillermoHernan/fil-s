/// <summary>
/// Exception class for compile errors.
/// </summary>

#pragma once

#include "scriptPosition.h"
#include "errorTypes.h"

#include <stdexcept>
#include <string>


/// <summary>
/// Exception class for compile errors.
/// </summary>
class CompileError : public std::logic_error
{
public:
	CompileError() : logic_error(""), m_type(ETYPE_OK)
	{
	}

	static CompileError create(const ScriptPosition& pos, ErrorTypes type, va_list args);

	const ScriptPosition& position()const
	{
		return m_position;
	}

private:
	CompileError(const std::string& text, const ScriptPosition& pos, ErrorTypes type) :
		logic_error(text), m_position(pos), m_type(type)
	{
	}

private:
	ErrorTypes		m_type;
    ScriptPosition	m_position;
};

void errorAt(const ScriptPosition& position, ErrorTypes type, ...);
void errorAt_v(const ScriptPosition& position, ErrorTypes type, va_list args);

