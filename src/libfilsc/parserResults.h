/// <summary>
/// Classes which define the possible results of parsing functions
/// </summary>

#pragma once

#include "RefCountObj.h"
#include "lexer.h"
#include "ast.h"
#include "compileError.h"


/**
 * Result of parsing an expression
 */
class ExprResult
{
public:
	Ref<AstNode> result;
    CompileError errorDesc;

    typedef ExprResult(*ParseFunction)(LexToken token);
    typedef ExprResult(*ChainParseFunction)(LexToken token, Ref<AstNode> prev);
    typedef bool (*TokenCheck)(LexToken token);

    ExprResult orElse(ParseFunction parseFn);
    ExprResult then(ParseFunction parseFn);
    ExprResult then(ChainParseFunction parseFn);

	ExprResult require(TokenCheck checkFn);
	ExprResult require(LEX_TYPES tokenType);
	ExprResult requireId(const char* text);
	ExprResult requireOp(const char* text);
	ExprResult requireReserved(const char* text);

	static ExprResult require(TokenCheck checkFn, LexToken token);
	static ExprResult require(LEX_TYPES tokenType, LexToken token);
	static ExprResult require(const char* text, LexToken token);
	static ExprResult requireReserved(const char* text, LexToken token);
	static ExprResult ok(LexToken token, Ref<AstNode> result);

	ExprResult skip();
	ExprResult getError(ErrorTypes type, ...);
	static ExprResult getError(LexToken token, ErrorTypes type, ...);

	std::string	nextText(int flags = LexToken::NONE)const;
	LexToken	nextToken(int flags = LexToken::NONE)const;
	LEX_TYPES	nextType(int flags = LexToken::NONE)const;

    bool error()const
    {
        return errorDesc.type() != ETYPE_OK;
    }
    
    bool ok()const
    {
        return !error();
    }

    void throwIfError()const;
    
    ExprResult final()const;

private:
	ExprResult(const LexToken &token, const Ref<AstNode> expr, const LexToken &initialToken)
		: m_token(token), result(expr), m_initialToken(initialToken)
	{
	}

	ExprResult(const LexToken &initialToken,
		const CompileError& err)
		: m_token(initialToken), errorDesc(err), m_initialToken(initialToken)
	{
	}

private:
	LexToken	m_token;
	LexToken    m_initialToken;
};
