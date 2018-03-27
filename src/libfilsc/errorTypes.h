/// <summary>
/// Contains the enumerated type with all compile errors.
/// </summary>
#pragma once

/// <summary>
/// Enumerated type with all supported compile errors. 
/// This single list is maintained to assign a code to each compile error.
/// </summary>
enum ErrorTypes
{
    ETYPE_OK = 0,
    ETYPE_NOT_IMPLEMENTED_1,
    ETYPE_UNEXPECTED_TOKEN_1,
    ETYPE_UNEXPECTED_TOKEN_2,
    ETYPE_INVALID_HEX_ESCAPE_SEQ,
    ETYPE_UNCLOSED_COMMENT,
    ETYPE_NEWLINE_IN_STRING,
    ETYPE_EOF_IN_STRING,
    ETYPE_INVALID_EXP_CHAIN,
    ETYPE_SYMBOL_ALREADY_DEFINED_1,
    ETYPE_NON_EXISTENT_SYMBOL_1,
    ETYPE_NOT_A_TYPE_1,
    ETYPE_INVALID_CODEGEN_NODE_1,
    ETYPE_DECLARATION_WITHOUT_TYPE,
    ETYPE_WRONG_IF_CONDITION_TYPE_1,
    ETYPE_MEMBER_NOT_FOUND_2,
    ETYPE_WRONG_TYPE_2,
    ETYPE_INCOMPATIBLE_TYPES_2,
    ETYPE_RECURSIVE_SYMBOL_REFERENCE_1,
    ETYPE_RETURN_OUTSIDE_FUNCTION,
    ETYPE_INCOMPATIBLE_RETURN_TYPE_2,
    ETYPE_MISPLACED_ACTOR_INSTANCE,
    ETYPE_RECURSIVE_ACTOR_INSTANCE,
    ETYPE_NON_CONST_ACTOR_INSTANCE,
    ETYPE_UNSPECIFIED_CONNECT_OUTPUT,
    ETYPE_INVALID_CONNECT_OUTPUT,
    ETYPE_CIRCULAR_MODULE_REFERENCE_1,
    ETYPE_MODULE_NOT_FOUND_1,
    ETYPE_WRITING_RESULT_FILE_2,
    ETYPE_ERROR_COMPILING_C_1,
    ETYPE_INVALID_COMPILE_SCRIPT_TEMPLATE_1,
    ETYPE_COMPILE_SCRIPT_TEMPLATE_NOT_FOUND_1,
    ETYPE_ERROR_LOADING_COMPILED_MODULE_1,
    ETYPE_NOT_CALLABLE,
    ETYPE_CANNOT_FIND_RUNTIME,
    ETYPE_BASE_DIR_NOT_CONFIGURED, 
    ETYPE_CODE_GENERATION_ERROR_1,

    //Add new error types above this line.
    //REMEMBER to add the description to 'errorTypeTemplate' function.
    //If the error description requires argument, add a '_x' sufix, where 'x' is the 
    //number of arguments. This is just a remainder to the programmer to supply the 
    //right number of parameters.

    //Errors under this line have no string assigned. They should be here temporarilly only

    ETYPE_COUNT		//Must be always the last one.
};
