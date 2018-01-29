/// <summary>
/// Exception class for compile errors.
/// </summary>

#pragma once

#include "scriptPosition.h"

#include <stdexcept>
#include <string>


/// <summary>
/// Exception class for compile errors.
/// </summary>
class CompileError : public std::logic_error
{
public:
	CompileError() : logic_error("")
	{
	}

    CompileError(const std::string &text, const ScriptPosition& pos) : 
    logic_error(text), m_position(pos)
    {
    }

	const ScriptPosition& position()const
	{
		return m_position;
	}

private:
    ScriptPosition		m_position;
};

void errorAt(const ScriptPosition& position, const char* msgFormat, ...);
void errorAt_v(const ScriptPosition& position, const char* msgFormat, va_list args);

