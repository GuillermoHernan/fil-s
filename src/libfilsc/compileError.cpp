/// <summary>
/// Exception class for compile errors.
/// </summary>

#include "pch.h"
#include "compileError.h"

using namespace std;


static string generateErrorMessage(const ScriptPosition* pPos, const char* msgFormat, va_list args)
{
    char buffer [2048];
    string message;

    if (pPos)
    {
        sprintf_s(buffer, "(line: %d, col: %d): ", pPos->line, pPos->column);
        message = buffer;
    }

    vsprintf_s(buffer, msgFormat, args);
    message += buffer;

    return message;
}

/**
 * Generates an error message located at the given position
 * @param code      Pointer to the code location where the error occurs. 
 * It is used to calculate line and column for the error message
 * @param msgFormat 'printf-like' format string
 * @param ...       Optional message parameters
 */
void errorAt(const ScriptPosition& position, const char* msgFormat, ...)
{
    va_list aptr;

    va_start(aptr, msgFormat);
    const std::string message = generateErrorMessage(&position, msgFormat, aptr);
    va_end(aptr);

    throw CompileError(message, position);
}

void errorAt_v(const ScriptPosition& position, const char* msgFormat, va_list args)
{
    const std::string message = generateErrorMessage(&position, msgFormat, args);

    throw CompileError(message, position);
}

