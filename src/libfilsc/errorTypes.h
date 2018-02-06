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

	//Add new error types above this line.
	//REMEMBER to add the description to 'errorTypeTemplate' function.
	//If the error description requires argument, add a '_x' sufix, where 'x' is the 
	//number of arguments. This is just a remainder to the programmer to supply the 
	//right number of parameters.

	//Errors under this line have no string assigned. They should be here temporarilly only
	ETYPE_SYMBOL_ALREADY_DEFINED_1,

	ETYPE_COUNT		//Must be always the last one.
};
