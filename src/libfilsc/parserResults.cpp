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
    
    ExprResult r = parseFn(m_token);
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

    ExprResult r = parseFn(nextToken());
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

    ExprResult r = parseFn(nextToken(), result);
    r.m_initialToken = m_initialToken;
    
    return r;    
}

/// <summary>
/// Requires that the next token complies with certain condition.
/// The 'condition' is a function which receives a token, and returns a boolean value.
/// </summary>
/// <param name="checkFn">Function used to validate the token.</param>
/// <returns>An expression result, which indicates the outcome of the operation.</returns>
ExprResult ExprResult::require(TokenCheck checkFn)
{
    if (error())
        return *this;

	auto r = require(checkFn, nextToken());
	r.m_initialToken = m_initialToken;
	return r;
}

/// <summary>
/// Requires that the token complies with certain condition.
/// The 'condition' is a function which receives a token, and returns a boolean value.
/// </summary>
/// <param name="checkFn">Function used to validate the token.</param>
/// <param name="token">Token to check</param>
/// <returns>An expression result, which indicates the outcome of the operation.</returns>
ExprResult ExprResult::require(TokenCheck checkFn, LexToken token)
{
	if (checkFn(token))
		return ExprResult (token, Ref<AstNode>(), token);
	else
		return getError(token, ETYPE_UNEXPECTED_TOKEN_1, token.text().c_str());
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

	auto r = require(tokenType, nextToken());
	r.m_initialToken = m_initialToken;
	return r;
}

/// <summary>Requires that the token has the specified type.</summary>
/// <param name="tokenType"></param>
/// <param name="token"></param>
/// <returns></returns>
ExprResult ExprResult::require(LEX_TYPES tokenType, LexToken token)
{
	if (token.type() == tokenType)
		return ExprResult(token, Ref<AstNode>(), token);
	else
		return getError(token, ETYPE_UNEXPECTED_TOKEN_2, 
			token.text().c_str(),
			tokenType2String(tokenType).c_str());
}

/// <summary>Requires that the token has the specified text.</summary>
/// <param name="tokenType"></param>
/// <param name="token"></param>
/// <returns></returns>
ExprResult ExprResult::require(const char* text, LexToken token)
{
	if (token.text() == text)
		return ExprResult(token, Ref<AstNode>(), token);
	else
		return getError(token, ETYPE_UNEXPECTED_TOKEN_2, token.text().c_str(), text);
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

	if (r.ok() && r.m_token.text() == text)
		return r;
	else
		return r.getError(ETYPE_UNEXPECTED_TOKEN_2, r.m_token.text().c_str(), text);
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

	if (r.ok() && r.m_token.text() == text)
		return r;
	else
		return r.getError(ETYPE_UNEXPECTED_TOKEN_2, r.m_token.text().c_str(), text);
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

	auto r = requireReserved(text, nextToken());
	r.m_initialToken = m_initialToken;
	return r;
}

ExprResult ExprResult::requireReserved(const char* text, LexToken token)
{
	ExprResult  r = require(LEX_RESERVED, token);

	if (r.ok() && r.m_token.text() == text)
		return r;
	else
		return r.getError(ETYPE_UNEXPECTED_TOKEN_2, token.text().c_str(), text);
}

/// <summary>
/// Creates a sucessful expression result.
/// </summary>
/// <param name="token">Token at which the result is located.</param>
/// <param name="result">AST node which is yielded as the result</param>
/// <returns></returns>
ExprResult ExprResult::ok(LexToken token, Ref<AstNode> result)
{
	return ExprResult(token, result, token);
}

/**
 * Skips next token. Forwards the previous result
 * @return 
 */
ExprResult ExprResult::skip()
{
    if (error())
        return *this;
    
    return ExprResult(nextToken(), result, m_initialToken);
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

	ExprResult result(m_token, CompileError::create(m_token.getPosition(), type, args));
	result.m_initialToken = m_initialToken;

	va_end(args);

	return result;
}

/// <summary>
/// Gets an error result located at the specified token.
/// </summary>
/// <param name="token"></param>
/// <param name="type"></param>
/// <param name=""></param>
/// <returns></returns>
ExprResult ExprResult::getError(LexToken token, ErrorTypes type, ...)
{
    va_list args;

    va_start(args, type);
    
	ExprResult result(token, CompileError::create(token.getPosition(), type, args));

	va_end(args);

    return result;
}

/// <summary>Gets the text of the next token.</summary>
/// <param name="flags">Flags to be passed to 'LexToken::next' function.</param>
/// <returns></returns>
std::string ExprResult::nextText(int flags)const
{
	return nextToken(flags).text();
}

/// <summary>Gets the next token.</summary>
/// <param name="flags">Flags to be passed to 'LexToken::next' function.</param>
/// <returns></returns>
LexToken ExprResult::nextToken(int flags)const
{
	return m_token.next(flags);
}

/// <summary>Gets the type of the next token.</summary>
/// <param name="flags">Flags to be passed to 'LexToken::next' function.</param>
/// <returns></returns>
LEX_TYPES ExprResult::nextType(int flags)const
{
	return nextToken(flags).type();
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
