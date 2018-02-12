/// <summary>
/// Exception class for compile errors.
/// </summary>

#include "pch.h"
#include "compileError.h"

using namespace std;

const char * errorTypeTemplate(ErrorTypes type);

/// <summary>
/// Generates a textual error message from an error type and a list of parameters.
/// </summary>
/// <param name="pPos"></param>
/// <param name="type"></param>
/// <param name="args"></param>
/// <returns></returns>
static string generateErrorMessage(const ScriptPosition* pPos, ErrorTypes type, va_list args)
{
    char buffer [2048];
    string message;
	const char * msgFormat = errorTypeTemplate(type);

    if (pPos)
    {
        sprintf_s(buffer, "(line: %d, col: %d): ", pPos->line, pPos->column);
        message = buffer;
    }

    vsprintf_s(buffer, msgFormat, args);
    message += buffer;

    return message;
}

/// <summary>
/// Creates an error message object located at the given position. 
/// </summary>
/// <param name="pos"></param>
/// <param name="type"></param>
/// <param name="args"></param>
/// <returns></returns>
CompileError CompileError::create(const ScriptPosition& pos, ErrorTypes type, va_list args)
{
	const string text = generateErrorMessage(&pos, type, args);

	return CompileError(text, pos, type);
}

/// <summary>
/// Creates a non-error compile error.
/// </summary>
/// <returns></returns>
CompileError CompileError::ok()
{
	return CompileError("", ScriptPosition(), ETYPE_OK);
}



/// <summary>
/// Generates an error message located at the given position
/// </summary>
/// <param name="position">Error position</param>
/// <param name="type">Error type</param>
/// <param name="">Optional message parameters. They depend on the type.</param>
void errorAt(const ScriptPosition& position, ErrorTypes type, ...)
{
    va_list aptr;

    va_start(aptr, type);
	errorAt_v(position, type, aptr);
    va_end(aptr);
}

/// <summary>
/// Generates an error message located at the given position. 
/// </summary>
/// <param name="position">Error position</param>
/// <param name="type">Error type</param>
/// <param name="args">Optional message parameters. They depend on the type.</param>
void errorAt_v(const ScriptPosition& position, ErrorTypes type, va_list args)
{
    throw CompileError::create(position, type, args);
}

/// <summary>
/// Gets the message template for a given error type
/// </summary>
/// <param name="type"></param>
/// <returns></returns>
const char * errorTypeTemplate(ErrorTypes type)
{
	static const char* templates[] = {
		/*ETYPE_OK*/					"Ok",
		/*ETYPE_NOT_IMPLEMENTED_1*/		"%s is not yet implemented",
		/*ETYPE_UNEXPECTED_TOKEN_1*/	"Unexpected token: '%s'",
		/*ETYPE_UNEXPECTED_TOKEN_2*/	"Unexpected token: '%s'. '%s' was expected.",
		/*ETYPE_INVALID_HEX_ESCAPE_SEQ*/"'\\x' escape sequence shall be followed by at least one hexadecimal digit",
		/*ETYPE_UNCLOSED_COMMENT*/		"Unclosed multi-line comment",
		/*ETYPE_NEWLINE_IN_STRING*/		"New line in string constant",
		/*ETYPE_EOF_IN_STRING*/			"End of file in string constant",
		/*ETYPE_INVALID_EXP_CHAIN*/		"Operators of different types cannot be chained. Group them using parenthesis. Example: x+(n-3)",
		/*ETYPE_SYMBOL_ALREADY_DEFINED_1*/"Symbol '%s' is already defined",
		/*ETYPE_NON_EXISTENT_SYMBOL_1*/	"Symbol '%s' does not exist",
		/*ETYPE_NOT_A_TYPE_1*/			"Symbol '%s' is not a data type",
		/*ETYPE_INVALID_CODEGEN_NODE_1*/"Code generation for nodes of type '%s' is not valid or not implemented",
	};

	return templates[type];
}
