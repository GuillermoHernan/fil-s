/// <summary>
/// Lexical analyzer for FIL-S language
/// </summary>

#pragma once

#include "scriptPosition.h"

enum LEX_TYPES
{
    LEX_EOF = 0,
    LEX_INITIAL,
    LEX_COMMENT,
    
    LEX_ID = 256,
    LEX_INT,
    LEX_FLOAT,
    LEX_STR,

	LEX_OPERATOR,
    
    LEX_R_LIST_END /* always the last entry */
};

/// To get the string representation of a token type
std::string tokenType2String(int token);


/**
 * Lexical analyzzer token. Tokens are the fragments in which input source is divided
 * and classified before being parsed.
 * 
 * The lexical analysis process is implemented taking a functional approach. There
 * is no 'lexer' object. There are functions which return the current state of the
 * lexer process as immutable 'LexToken' objects. 
 * These objects are not strictly 'immutable', as they have assignment operator. But none
 * of their public methods modify its internal state.
 */
class LexToken
{
public:

    /// <summary>Constructor which receives a source code string.</summary>
    /// <remarks>
    /// The constructor doesn't make a copy of the input string, so it is important
	/// not to delete input string while there are still live 'LexToken's using it.
	/// 
	/// The token created with the constructor is not parsed from input string. It is
	/// just the 'initial' token.To parse the first real token, call 'next'.
    /// </remarks>
    LexToken(const char* code);

    LexToken(LEX_TYPES lexType, const char* code, const ScriptPosition& position, int length);

    /// Reads next token from input, and returns it.
    LexToken next(bool skipComments = true)const;

    /// Checks that the current token matches the expected, and returns next
	LexToken match(int expected_tk)const;
	LexToken match(int expected_tk, const char* expected_text)const;

    ///Return a string representing the position in lines and columns of the token

    const ScriptPosition& getPosition()const
    {
        return m_position;
    }

    LEX_TYPES type()const
    {
        return m_type;
    }
    
    bool eof()const
    {
        return m_type == LEX_EOF;
    }
    std::string text()const;

    const char* code()const
    {
        return m_code;
    }

    std::string strValue()const;

	bool isOperator(const char* opText)const;

private:
    const char*		m_code;			//Pointer to the position of the token in the source string.
    LEX_TYPES		m_type;			//Token type.
	int				m_length;		//Token length.
	ScriptPosition	m_position;

    LexToken nextDispatch()const;

    LexToken buildNextToken(LEX_TYPES lexType, const char* code, int length)const;
    LexToken parseComment(const char * code)const;
    LexToken parseId(const char * code)const;
    LexToken parseNumber(const char * code)const;
    LexToken parseString(const char * code)const;
    LexToken parseOperator(const char * code)const;

    ScriptPosition calcPosition(const char* code)const;
    LexToken errorAt(const char* charPos, const char* msgFormat, ...)const;
};