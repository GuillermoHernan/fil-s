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
    char buffer[2048];
    string message;
    const char * msgFormat = errorTypeTemplate(type);

    if (pPos)
    {
        sprintf_s(buffer, "(line: %d, col: %d): ", pPos->line(), pPos->column());
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
/// Creates an error message object located at the given position. 
/// </summary>
/// <param name="pos"></param>
/// <param name="type"></param>
/// <param name=""></param>
/// <returns></returns>
CompileError CompileError::create(const ScriptPosition& pos, ErrorTypes type, ...)
{
    va_list aptr;

    va_start(aptr, type);
    auto result = create(pos, type, aptr);
    va_end(aptr);

    return result;
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
        /*ETYPE_DECLARATION_WITHOUT_TYPE*/"Variable declaration must have a type or an initialization expression",
        /*ETYPE_WRONG_IF_CONDITION_TYPE_1*/"'if' conditions must be of type 'bool', not '%s'",
        /*ETYPE_MEMBER_NOT_FOUND_2,*/	"Member '%s' not found in type '%s'",
        /*ETYPE_WRONG_TYPE_2*/			"wrong type '%s' in expression. '%s' was expected.",
        /*ETYPE_INCOMPATIBLE_TYPES_2*/	"Type '%s' is incompatible and cannot be assigned to type '%s'",
        /*ETYPE_RECURSIVE_SYMBOL_REFERENCE_1*/ "Symbol '%s' is referenced in its initialization expression",
        /*ETYPE_RETURN_OUTSIDE_FUNCTION*/"Return statements can only be used inside functions",
        /*ETYPE_INCOMPATIBLE_RETURN_TYPE_2*/"Returned type '%s' is incompatible with function return type '%s'",
        /*ETYPE_MISPLACED_ACTOR_INSTANCE*/"Actor instances can only exist inside other actors",
        /*ETYPE_RECURSIVE_ACTOR_INSTANCE*/"The actor instance is of the same, or contains, the container actor",
        /*ETYPE_NON_CONST_ACTOR_INSTANCE*/"Actor instances must be contant ('const')",
        /*ETYPE_UNSPECIFIED_CONNECT_OUTPUT*/"No output specified in connect expression",
        /*ETYPE_INVALID_CONNECT_OUTPUT*/"Invalid output for connect expression",
        /*ETYPE_CIRCULAR_MODULE_REFERENCE_1*/"Circular module reference detected in module '%s'",
        /*ETYPE_MODULE_NOT_FOUND_1*/	"Module '%s' not found",
        /*ETYPE_WRITING_RESULT_FILE_2*/	"Error writing results file (%s): %s",
        /*ETYPE_ERROR_COMPILING_C_1*/	"Error compiling 'C' code in module: %s",
        /*ETYPE_INVALID_COMPILE_SCRIPT_TEMPLATE_1*/"Invalid 'C' compile script template: %s",
        /*ETYPE_COMPILE_SCRIPT_TEMPLATE_NOT_FOUND_1*/"Cannot find 'C' compile script template at: %s",
        /*ETYPE_ERROR_LOADING_COMPILED_MODULE_1*/"Error loading compiled module at: %s",
        /*ETYPE_NOT_CALLABLE*/			"The expression does not evaluate to a callable object",
        /*ETYPE_CANNOT_FIND_RUNTIME*/   "Cannot find FIL-S runtime. Possibly, compiler install is corrupted.",
    };

    return templates[type];
}
