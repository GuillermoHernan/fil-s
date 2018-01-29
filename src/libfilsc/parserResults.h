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
    LexToken token;
    Ref<AstNode> result;
    CompileError errorDesc;

    ExprResult(const LexToken &nextToken,
                    const Ref<AstNode> expr)
    : token(nextToken), result(expr), m_initialToken(token)
    {
    }

    ExprResult(const LexToken &initialToken,
                    const CompileError& err)
    : token(initialToken), errorDesc(err), m_initialToken(initialToken)
    {
    }

    ExprResult(const LexToken &initialToken)
    : token(initialToken), m_initialToken(token)
    {
    }

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
	ExprResult requireSeparator(const char* separator, const char* ending);
	ExprResult readId(std::string* name);
	ExprResult skip();
    ExprResult getError(ErrorTypes type, ...);

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
    LexToken    m_initialToken;
};
