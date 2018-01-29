/// <summary>
/// Classes which define the possible results of parsing functions
/// </summary>

#include "pch.h"
#include "parserResults.h"
#include "utils.h"
#include "CompileError.h"

/**
 * If the current result is an error, executes the parse function.
 * If not, it doesn't call the parse function.
 * @param parseFn
 * @return 
 */
ExprResult ExprResult::orElse(ParseFunction parseFn)
{
    if (ok())
        return *this;
    
    ExprResult r = parseFn(token);
    r.m_initialToken = m_initialToken;
    
    if (r.ok() || r.errorDesc.position() > this->errorDesc.position())
        return r;
    else
        return *this;
}

/**
 * If the current result is a success, executes the parse function and returns
 * its result.
 * Version for regular parse functions.
 * @param parseFn
 * @return 
 */
ExprResult ExprResult::then(ParseFunction parseFn)
{
    if (error())
        return *this;

    ExprResult r = parseFn(token);
    r.m_initialToken = m_initialToken;
    
    return r;    
}

/**
 * If the current result is a success, executes the parse function and returns
 * its result.
 * Version for chained parse functions. Chained parse functions receive the current
 * result as parameter.
 * @param parseFn
 * @return 
 */
ExprResult ExprResult::then(ChainParseFunction parseFn)
{
    if (error())
        return *this;

    ExprResult r = parseFn(token, result);
    r.m_initialToken = m_initialToken;
    
    return r;    
}

/**
 * Requires that the current token complies with certain condition.
 * The 'condition' is a function which receives a token, and return a boolean
 * value.
 * If the check is successful, advances to the next token in the result.
 * @param checkFn   Token check function.
 * @return 
 */
ExprResult ExprResult::require(TokenCheck checkFn)
{
    if (error())
        return *this;
    
    if (checkFn(token))
    {
        ExprResult r (token.next(), result);
        r.m_initialToken = m_initialToken;
        return r;
    }
    else
        return getError(ETYPE_UNEXPECTED_TOKEN_1, token.text().c_str());
}

/**
 * Requires that the current token is of the specified type.
 * If the check is successful, advances to the next token in the result.
 * @param tokenType     Token type
 * @return 
 */
ExprResult ExprResult::require(LEX_TYPES tokenType)
{
    if (error())
        return *this;
    
    if (token.type() == tokenType)
    {
        ExprResult r (token.next(), result);
        r.m_initialToken = m_initialToken;
        return r;
    }
    else
        return getError(ETYPE_UNEXPECTED_TOKEN_2, 
			token.text().c_str(), 
			tokenType2String(tokenType).c_str());
}

/**
 * Requires that the current token is an identifier with the given text.
 * @param text
 * @return 
 */
ExprResult ExprResult::requireId(const char* text)
{
	if (error())
		return *this;

	ExprResult  r = require(LEX_ID);

	if (r.ok() && token.text() == text)
		return r;
	else
		return getError(ETYPE_UNEXPECTED_TOKEN_2, token.text().c_str(), text);
}

/// <summary>
/// Requires that the current token is the specified operator.
/// </summary>
/// <param name="text">Operator text</param>
/// <returns></returns>
ExprResult ExprResult::requireOp(const char* text)
{
	if (error())
		return *this;

	ExprResult  r = require(LEX_OPERATOR);

	if (r.ok() && token.text() == text)
		return r;
	else
		return getError(ETYPE_UNEXPECTED_TOKEN_2, token.text().c_str(), text);
}

/// <summary>
/// Requires that the current token is the specified reserved word.
/// </summary>
/// <param name="text"></param>
/// <returns></returns>
ExprResult ExprResult::requireReserved(const char* text)
{
	if (error())
		return *this;

	ExprResult  r = require(LEX_RESERVED);

	if (r.ok() && token.text() == text)
		return r;
	else
		return getError(ETYPE_UNEXPECTED_TOKEN_2, token.text().c_str(), text);
}

/**
 * Skips next token. Forwards the previous result
 * @return 
 */
ExprResult ExprResult::skip()
{
    if (error())
        return *this;
    
    ExprResult  r(token.next(), result);
    r.m_initialToken = m_initialToken;
    
    return r;    
}


/// <summary>
/// Gets an error result located at the current token.
/// </summary>
/// <param name="type"></param>
/// <param name=""></param>
/// <returns></returns>
ExprResult ExprResult::getError(ErrorTypes type, ...)
{
    va_list args;

    va_start(args, type);
    
	ExprResult result(token, CompileError::create(token.getPosition(), type, args));
    result.m_initialToken = m_initialToken;

	va_end(args);

    return result;
    
}

/**
 * Throws a script exception if it is an error result.
 */
void ExprResult::throwIfError()const
{
	if (error())
		throw errorDesc;
}

/**
 * If the result is an error, relocates it to the initial token
 * @return 
 */
ExprResult ExprResult::final()const
{
    if (ok())
        return *this;
    else
        return ExprResult(m_initialToken, errorDesc);
}
