/// <summary>
/// Parses FIL-S code into an Abstract Sintax Tree (AST) structure.
/// </summary>

#include "pch.h"
#include "parser_internal.h"
#include "utils.h"

using namespace std;

/// <summary>
/// Checks if the token is an assignment operator.
/// </summary>
/// <param name="token"></param>
/// <returns></returns>    
bool isAssignment(LexToken token)
{
	static set<string> operators;

	if (operators.empty())
	{
		operators.insert("=");
		operators.insert(">>>=");
		operators.insert(">>=");
		operators.insert("<<=");
		operators.insert("**=");
		operators.insert("+=");
		operators.insert("-=");
		operators.insert("*=");
		operators.insert("/=");
		operators.insert("%=");
		operators.insert("&=");
		operators.insert("|=");
		operators.insert("^=");
	}

	return (token.type() == LEX_OPERATOR && operators.count(token.text()) > 0);
}

/// <summary>
/// Checks if the operator is a binary operator.
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
bool isBinaryOp(LexToken token)
{
	static set<string> operators;

	if (operators.empty())
	{
		operators.insert(">>>");
		operators.insert(">>");
		operators.insert("<<");
		operators.insert("**");
		operators.insert("+");
		operators.insert("-");
		operators.insert("*");
		operators.insert("/");
		operators.insert("%");
		operators.insert("&");
		operators.insert("|");
		operators.insert("&&");
		operators.insert("||");
		operators.insert("^");
		operators.insert("<");
		operators.insert(">");
		operators.insert(">=");
		operators.insert("<=");
		operators.insert("==");
		operators.insert("!=");
	}

	return (token.type() == LEX_OPERATOR && operators.count(token.text()) > 0);
}

/// <summary>
/// Checks if the token is a prefix operator
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
bool isPrefixOp(LexToken token)
{
	static set<string> operators;

	if (operators.empty())
	{
		operators.insert("-");
		operators.insert("+");
		operators.insert("--");
		operators.insert("++");
		operators.insert("!");
		operators.insert("~");
	}

	return (token.type() == LEX_OPERATOR && operators.count(token.text()) > 0);
}

/// <summary>
/// Checks if the token is a postfix operator
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
bool isPostfixOp(LexToken token)
{
	static set<string> operators;

	if (operators.empty())
	{
		operators.insert("--");
		operators.insert("++");
	}

	return (token.type() == LEX_OPERATOR && operators.count(token.text()) > 0);
}


//bool oneOf (LexToken token, const char* chars)
//{
//    const LEX_TYPES t = token.type();
//    
//    if ( t >= LEX_ASSIGN_BASE )
//        return false;
//    
//    for (; *chars != 0; ++chars)
//        if (*chars == (char)t)
//            return true;
//    
//    return false;
//}

//bool oneOf (LexToken token, const int* ids)
//{
//    const LEX_TYPES t = token.type();
//    
//    for (; *ids != 0; ++ids)
//        if (*ids == t)
//            return true;
//    
//    return false;
//}

/**
 * Crates an empty statement
 * @return 
 */
//Ref<AstNode> emptyStatement(ScriptPosition pos)
//{
//    return AstLiteral::createNull(pos);
//}

/// <summary>
/// Parses a list node.
/// </summary>
/// <param name="token">Initial token</param>
/// <param name="itemParseFn">Function to parse each item node</param>
/// <param name="beginTok">Text of the begin token. Optional, use an empty string to
/// not use a begin token.</param>
/// <param name="endTok">Text of the end token. Mandatory</param>
/// <param name="separator">Text of the separator token. Optional, use an empty string to
/// not use a separator token.</param>
/// <returns></returns>
ExprResult parseList(LexToken token, ExprResult::ParseFunction itemParseFn,
	const char* beginTok, const char* endTok, const char* separator)
{
	ExprResult	initial(token);
	ExprResult	r(initial);
	auto		result = refFromNew(new AstBranchNode(AST_LIST, token.getPosition()));

	if (*beginTok)
		r = r.requireOp(beginTok);

	while (r.ok() && r.token.text() != endTok)
	{
		r = r.then(itemParseFn);

		if (r.ok())
			result->addChild(r.result);

		if (*separator && r.token.text() != endTok)
			r = r.requireOp(separator);
	}

	r = r.requireOp(endTok);

	if (r.ok())
		r.result = result;

	return r;
}


/// <summary>
/// Parses a script.
/// </summary>
/// <param name="script">text string which contains the complete script.</param>
/// <returns></returns>
ExprResult parseScript(const char* script)
{
	LexToken tok(script);

	return parseScript(tok.next());
}


/**
 * Parses an script, which is a list of statements
 * @param token
 * @return 
 */
ExprResult parseScript(LexToken token)
{
    auto script = astCreateScript(token.getPosition());
    
    while (!token.eof())
    {
        const ExprResult   parseRes = parseTopLevelItem(token);

		if (parseRes.error())
			return parseRes;

        script->addChild(parseRes.result);
        token = parseRes.token;
    }

    return ExprResult(token, script);
}

/// <summary>
/// Parses a top-level item. An item which is not part of another language construct.
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseTopLevelItem(LexToken token)
{
	return parseConst(token).orElse(parseActorExpr).orElse(parseFunctionDef);
}


/**
 * Parses one statement
 * @param token
 * @return 'ExprResult' structure, with the generated AST tree, and a pointer to
 * the next token.
 */
//ExprResult parseStatement (LexToken token)
//{
//    switch ((int)token.type())
//    {
//    case '{':
//    {
//        ExprResult  r = parseObjectLiteral(token);
//        
//        if (r.ok())
//            return r.toParseResult();
//        else
//            return parseBlock(token).toParseResult();
//    }
//    case ';':           return ExprResult (token.next(), emptyStatement(token.getPosition()));
//    case LEX_R_VAR:
//    case LEX_R_CONST:
//        return parseDeclaration (token).toParseResult();
//    case LEX_R_IF:      return parseIf (token);
//    case LEX_R_WHILE:   return parseWhile (token);
//    case LEX_R_FOR:     return parseFor (token);
//    case LEX_R_RETURN:  return parseReturn (token);
//    case LEX_R_FUNCTION:return parseFunctionExpr (token).toParseResult();
//    case LEX_R_ACTOR:   return parseActorExpr (token).toParseResult();
//    case LEX_R_CLASS:   return parseClassExpr (token).toParseResult();
//    case LEX_R_EXPORT:  return parseExport (token).toParseResult();
//    case LEX_R_IMPORT:  return parseImport (token).toParseResult();
//    
//    default:
//        return parseSimpleStatement(token);
//    }//switch
//}

/**
 * Parses a simple statement (no blocks, no reserved words)
 * @param token
 * @return 
 */
//ExprResult parseSimpleStatement (LexToken token)
//{
//    switch ((int)token.type())
//    {
//    case LEX_ID:
//    case LEX_INT:
//    case LEX_FLOAT:
//    case LEX_STR:
//    case '-':
//    case '+':
//    case '!':
//    case '~':
//    case LEX_PLUSPLUS:
//    case LEX_MINUSMINUS:
//    {
//        ExprResult r = parseExpression (token);
//        return r.toParseResult();
//    }
//    case LEX_R_VAR:
//    case LEX_R_CONST:
//        return parseDeclaration (token).toParseResult();
//    default:
//        errorAt(token.getPosition(), "Unexpected token: '%s'", token.text().c_str());
//        return ExprResult (token.next(), emptyStatement(token.getPosition()));
//    }//switch
//}
//
/**
 * Parses an statement which is part of the body of an 'if' or loop
 * @param token
 * @return 
 */
//ExprResult parseBodyStatement (LexToken token)
//{
//    ExprResult  r(token);
//    
//    if (token.type() == ';')
//    {
//        r = r.skip();
//        r.result = emptyStatement(token.getPosition());
//    }
//    else
//    {
//        ExprResult     result = parseStatement(token);
//        
//        switch ((int)token.type())
//        {
//        case '{':
//        case LEX_R_IF:
//        case LEX_R_WHILE:
//        case LEX_R_FOR:
//        case LEX_R_FUNCTION:
//            break;
//            
//        default:
//            //with all other statements, it may follow a semicolon.
//            if (result.nextToken.type() == ';')
//                result.nextToken = result.nextToken.next();
//        }
//        
//        r.token = result.nextToken;
//        r.result = result.ast;
//    }//else.
//    
//    return r.final();
//}


/**
 * Parses a code block
 * @param token
 * @return Code block object
 */

/// <summary>Parses a block expression.</summary>
/// <remarks>
/// A block expression is a list of expressions delimited by brackets {...},
/// which yields the value of the last expression on the list.
/// </remarks>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseBlock (LexToken token)
{
    Ref<AstNode>    block = astCreateBlock(token);
    ExprResult      r(token);
    
    r = r.requireOp("{");
    
    while (r.ok() && !r.token.isOperator("}"))
    {
		r = r.then(parseExpression).orElse(parseVar).orElse(parseConst);
		block->addChild(r.result);

		//Skip optional ';' separators.
		while (r.ok() && r.token.isOperator(";"))
			r = r.skip();
    }
    
    r = r.requireOp("}");
    
    if (r.ok())
        r.result = block;
    
    return r.final();
}

/**
 * Parses a 'var' declaration
 * @param token
 * @return 
 */

/// <summary>
/// Parses a declaration. It does not parse the modifiers. They should be parsed
/// by the calling function.
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseDeclaration (LexToken token)
{
	ExprResult      initial(token);
	ExprResult      r(initial);
	Ref<AstNode>	typeDescriptor;
	Ref<AstNode>	initExp;

	r = r.require(LEX_ID);
	if (!r.ok())
		return r.final();

	//Type descriptor is optional.
	if (r.token.isOperator(":"))
	{
		r = r.then(parseTypeSpecifier);
		typeDescriptor = r.result;
	}

	//Initialization is also optional.
	if (r.token.isOperator("="))
	{
		r = r.skip().then(parseExpression);

		initExp = r.result;
	}

	if (r.ok())
		r.result = astCreateDeclaration(initial.token, typeDescriptor, initExp);

	return r;
}

/// <summary>
/// Parses any declaration: 'var', 'const' or not specified (default access)
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseAnyDeclaration(LexToken token)
{
	return parseConst(token).orElse(parseVar).orElse(parseDeclaration);
}

/// <summary>
/// Parses a constant definition expression
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseConst(LexToken token)
{
	ExprResult      r(token);

	r = r.requireReserved("const").then(parseDeclaration);

	if (r.ok())
		r.result->changeType(AST_CONST);

	return r.final();
}

/// <summary>
/// Parses a variable definition expression
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseVar(LexToken token)
{
	ExprResult      r(token);

	r = r.requireReserved("var").then(parseDeclaration);

	if (r.ok())
		r.result->changeType(AST_VAR);

	return r.final();
}

/// <summary>
/// Parses a type specifier for a declaration.
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseTypeSpecifier(LexToken token)
{
	ExprResult      initial(token);

	return initial.requireOp(":").then(parseTypeDescriptor);
}

/// <summary>
/// Parses a type descriptor, which can be just a type name or a tuple/structure
/// declaration.
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseTypeDescriptor(LexToken token)
{
	return parseIdentifier(token).orElse(parseTupleDef);
}

/// <summary>
/// Parses a tuple definition
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseTupleDef(LexToken token)
{
	ExprResult r = parseList(token, parseTupleDefItem, "(", ")", ",");

	if (r.ok())
		r.result->changeType(AST_TUPLE_DEF);

	return r.final();
}

/// <summary>
/// Parses on of the type of a tuple / struct.
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseTupleDefItem(LexToken token)
{
	return parseAnyDeclaration(token).orElse(parseTypeDescriptor);
}

/// <summary>
/// Parses an 'if' flow control expression.
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseIf(LexToken token)
{
	ExprResult	r(token);

	r = r.requireReserved("if").requireOp("(").then(parseExpression);

	auto conditionExpr = r.result;

	r = r.requireOp(")").then(parseExpression);

	auto			thenExpr = r.result;

	//A single semicolon may follow 'then' expression.
	if (r.ok() && r.token.isOperator(";"))
		r = r.skip();

	Ref<AstNode>	elseExpr;

	//Check for the presence of 'else'
	if (r.ok() && r.token.text() == "else")
	{
		r = r.requireReserved("else").then(parseExpression);
		thenExpr = r.result;
	}

	if (r.ok())
		r.result = astCreateIf(token.getPosition(), conditionExpr, thenExpr, elseExpr);

	return r.final();
}

/// <summary>
/// Parses a 'select' expression
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseSelect(LexToken token)
{
	ExprResult	r(token);

	r = r.requireReserved("select").requireOp("(").then(parseExpression);

	if (r.ok())
		r = r.getError(ETYPE_NOT_IMPLEMENTED_1, "'select' parsing");

	return r.final();
}


/**
 * Parses an 'if' statement
 * @param token
 * @return 
 */
//ExprResult parseIf (LexToken token)
//{
//    ScriptPosition  pos = token.getPosition();
//
//    token = token.match(LEX_R_IF);
//    token = token.match('(');
//    
//    ExprResult rCondition = parseExpression(token);
//    rCondition.throwIfError();
//
//    token = rCondition.token;
//    
//    ExprResult r = parseBodyStatement(token.match(')')).toParseResult();
//    Ref<AstNode>   thenSt = r.ast;
//    Ref<AstNode>   elseSt;
//    
//    token = r.nextToken;
//    
//    if (token.type() == LEX_R_ELSE)
//    {
//        r = parseBodyStatement(token.next()).toParseResult();
//        elseSt = r.ast;
//        token = r.nextToken;
//    }
//    
//    auto result = astCreateIf (pos, rCondition.result, thenSt, elseSt);
//    
//    return ExprResult (token, result);
//}

/**
 * Parses a 'while' statement
 * @param token
 * @return 
 */
//ExprResult parseWhile (LexToken token)
//{
//    ScriptPosition  pos = token.getPosition();
//
//    token = token.match(LEX_R_WHILE);
//    token = token.match('(');
//    
//    ExprResult rCondition = parseExpression(token);
//    rCondition.throwIfError();
//
//    token = rCondition.token;
//    
//    ExprResult r = parseBodyStatement(token.match(')')).toParseResult();
//    
//    auto result = astCreateFor (pos, 
//                                Ref<AstNode>(), 
//                                rCondition.result, 
//                                Ref<AstNode>(), 
//                                r.ast);
//    
//    return ExprResult (r.nextToken, result);
//}

/**
 * Parses a 'for' statement
 * @param token
 * @return 
 */
//ExprResult parseFor (LexToken token)
//{
//    ExprResult  res = parseForEach(token);
//    
//    if (res.ok())
//        return res.toParseResult();
//    
//    //TODO: Better handling of ';'
//    ScriptPosition      pos = token.getPosition();
//    Ref<AstNode>   init;
//    Ref<AstNode>   increment;
//    Ref<AstNode>   body;
//    Ref<AstNode>  condition;
//
//    token = token.match(LEX_R_FOR);
//    token = token.match('(');
//    
//    ExprResult r;
//    
//    //Initialization
//    if (token.type() != ';')
//    {
//        r = parseSimpleStatement(token);
//        init = r.ast;
//        token = r.nextToken.match(';');
//    }
//    else
//        token = token.next();
//    
//    ExprResult rCondition = parseExpression(token);
//    rCondition.throwIfError();
//    condition = rCondition.result;
//    token = rCondition.token.match(';');
//    
//    if (token.type() != ')')
//    {
//        r = parseSimpleStatement(token);
//        increment = r.ast;
//        token = r.nextToken;        
//    }
//    
//    token = token.match(')');
//    r = parseBodyStatement(token).toParseResult();
//    body = r.ast;
//    token = r.nextToken;
//
//    auto result = astCreateFor (pos, init, condition, increment, body);
//    
//    return ExprResult (token, result);
//}

/**
 * Parses a for loop which iterates over the elements of a sequence. 
 * 'for (x in z)...'
 * @param token
 * @return 
 */
//ExprResult parseForEach (LexToken token)
//{
//    ExprResult  r(token);
//    
//    r = r.require(LEX_R_FOR).require('(');
//    if (r.error())
//        return r.final();
//
//    r = r.then(parseIdentifier).requireId("in");
//    if (r.error())
//        return r.final();
//    
//    auto itemDecl = r.result;
//    r = r.then(parseExpression).require(')');
//    
//    if (r.error())
//        return r.final();
//    
//    auto seqExpression = r.result;
//    
//    r = r.then (parseBodyStatement);
//    
//    if (r.ok())
//        r.result = astCreateForEach (token.getPosition(), itemDecl, seqExpression, r.result);
//    
//    return r.final();
//}


/**
 * Parses a return statement
 * @param token
 * @return 
 */
//ExprResult parseReturn (LexToken token)
//{
//    ScriptPosition      pos = token.getPosition();
//    Ref<AstNode>  result;
//
//    token = token.match(LEX_R_RETURN);
//    
//    if (token.type() == ';')
//        token = token.next();
//    else
//    {
//        ExprResult r = parseExpression(token);
//        r.throwIfError();
//        
//        token = r.token;
//        result = r.result;
//        if (token.type() == ';')
//            token = token.next();
//    }
//    
//    auto  ret = astCreateReturn(pos, result);
//    return ExprResult(token, ret);    
//}

/**
 * Parses a function argument list.
 * @param token 
 * @param fnNode  Function into which add the arguments
 * @return  ExprResult containing the same function AST node received, with
 * the parameters added.
 */
//ExprResult parseArgumentList(LexToken token, Ref<AstNode> fnNode)
//{
//    ExprResult          r(token);
//    
//    r = r.require('(');
//
//    while (r.ok() && r.token.type() != ')')
//    {
//        const string name = r.token.text();
//
//        r = r.require (LEX_ID);
//        if (r.ok())
//            fnNode->addParam(name);
//
//        if (r.token.type() != ')')
//            r = r.require (',');
//    }
//    
//    r = r.require(')');
//    
//    if (r.ok())
//        r.result = fnNode;
//    
//    return r.final();
//}

/// <summary>
/// Parses any valid expression.
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseExpression (LexToken token)
{
    return parseAssignment (token)
		.orElse(parseBinaryExpr)
		.orElse(parsePrefixExpr)
		.orElse(parsePostfixExpr)
		.orElse(parseTerm);
}

/// <summary>
/// Parses a term. A 'term' is a component of a mathematical or logical expression
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseTerm(LexToken token)
{
	return parseConditional(token)
		.orElse(parseLeftExpr);
}


/// <summary>
/// Parses an assignment expression.
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseAssignment (LexToken token)
{
    ExprResult r = parseLeftExpr(token);

    auto	lexpr = r.result;
	auto	opToken = r.token;

    r = r.require(isAssignment).then (parseExpression);
    
	if (r.ok())
        r.result = astCreateAssignment(opToken, lexpr, r.result);

	return r.final();
}

/// <summary>
/// Parses an expression which can be at the left side of an assignment
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseLeftExpr (LexToken token)
{
	return parsePostfixExpr(token).orElse(parsePrimaryExpr);
}


/// <summary>
/// PArses a binary operator expression
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseBinaryExpr(LexToken token)
{
	ExprResult	r = parseTerm(token);
	auto		leftExpr = r.result;
	auto		opToken = r.token;

	r = r.require(isBinaryOp).then(parseTerm);
	if (r.ok())
	{
		auto result = astCreateBinaryOp(opToken, leftExpr, r.result);

		//Parse chained operations.
		while (r.ok() && r.token.text() == opToken.text())
		{
			opToken = r.token;
			r = r.skip().then(parseTerm);

			if (r.ok())
				result = astCreateBinaryOp(opToken, result, r.result);
		}

		if (r.ok() && isBinaryOp(r.token))
			r = r.getError(ETYPE_INVALID_EXP_CHAIN);
	}

	return r.final();
}

/// <summary>
/// Parses a prefix expression
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parsePrefixExpr(LexToken token)
{
	ExprResult	r(token);

	r = r.require(isPrefixOp).then(parseTerm);

	if (r.ok())
		r.result = astCreatePrefixOp(token, r.result);

	return r.final();
}

/// <summary>
/// Parses a postfix expression
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parsePostfixExpr (LexToken token)
{
	ExprResult	r = parsePrimaryExpr(token);

	while (r.ok())
	{
		string	opText = r.token.text();

		if (opText == ".")
			r = r.then(parseMemberAccess);
		else if (opText == "(")
			r = r.then(parseCallExpr);
		if (isPostfixOp(r.token))
			r = r.then(parsePostfixOperator);
		else
			break;
	}

	return r.final();
}

/// <summary>
/// PArses a postfix operator
/// </summary>
/// <param name="token"></param>
/// <param name="termExpr"></param>
/// <returns></returns>
ExprResult parsePostfixOperator(LexToken token, Ref<AstNode> termExpr)
{
	ExprResult	r(token);

	r = r.require(isPostfixOp);
	if (r.ok())
		r.result = astCreatePostfixOp(token, termExpr);

	return r.final();
}

/// <summary>
/// Parses a function call expression
/// </summary>
/// <param name="token"></param>
/// <param name="fnExpr"></param>
/// <returns></returns>
ExprResult parseCallExpr(LexToken token, Ref<AstNode> fnExpr)
{
	ExprResult	r = parseTuple(token);

	if (r.ok())
		r.result = astCreateFnCall(token.getPosition(), fnExpr, r.result);

	return r.final();
}

/// <summary>
/// Parses a literal value
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseLiteral(LexToken token)
{
	Ref<AstLiteral>		value;

	switch ((int)token.type())
	{
	case LEX_ID:
		if (token.text() == "true")
			value = AstLiteral::createBool(token.getPosition(), true);
		else if (token.text() == "false")
			value = AstLiteral::createBool(token.getPosition(), false);
		break;

	case LEX_FLOAT:
	case LEX_INT:
	case LEX_STR:
		value = AstLiteral::create(token);
		break;

	default:
		break;
	}//switch

	if (value.notNull())
		return ExprResult(token.next(), value);
	else
		return ExprResult(token).getError(ETYPE_UNEXPECTED_TOKEN_2, token.text().c_str(), "literal");
}


/// <summary>
/// Parses a expression between parenthesis '(...)'.
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseParenthesisExpr(LexToken token)
{
	ExprResult r(token);

	return r.requireOp("(").then(parseExpression).requireOp(")").final();
}

/// <summary>
/// Parses a tuple creation '(a,b,c...)'
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseTuple(LexToken token)
{
	ExprResult		r(token);
	Ref<AstNode>	result = astCreateTuple(token);

	r = r.requireOp("(");

	if (!r.token.isOperator(")"))
	{
		r = r.then(parseExpression);
		result->addChild(r.result);

		while (r.ok() && !r.token.isOperator(")"))
		{
			r = r.requireOp(",").then(parseExpression);
			result->addChild(r.result);
		}
	}

	r = r.requireOp(")");
	if (r.ok())
		r.result = result;

	return r.final();
}

/// <summary>
/// Parses a conditional expression
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseConditional(LexToken token)
{
	return parseIf(token).orElse(parseSelect);
}

/**
 * Parses a conditional expression ('?' operator)
 * @param token
 * @return 
 */
//ExprResult parseConditional (LexToken token)
//{
//    ExprResult      r = parseLogicalOrExpr (token);
//    
//    if (r.ok() && r.token.type() == '?')
//    {
//        const Ref<AstNode>  condition =r.result;
//        
//        r = r.skip();
//        r = r.then (parseAssignment).require(':');
//        const Ref<AstNode>  thenExpr =r.result;
//        
//        r = r.then (parseAssignment);
//        const Ref<AstNode>  elseExpr =r.result;
//        
//        if (r.ok())
//            r.result = astCreateConditional (token.getPosition(),
//                                               condition,
//                                               thenExpr,
//                                               elseExpr);
//        
//    }
//    
//    return r.final();
//}

///**
// * Parses a 'logical OR' expression ('||' operator)
// * @param token
// * @return 
// */
//ExprResult parseLogicalOrExpr (LexToken token)
//{
//    return parseBinaryLROp(token, LEX_OROR, parseLogicalAndExpr);
//}
//
///**
// * Parses a 'logical AND' expression ('&&' operator)
// * @param token
// * @return 
// */
//ExprResult parseLogicalAndExpr (LexToken token)
//{
//    return parseBinaryLROp(token, LEX_ANDAND, parseBitwiseOrExpr);
//}
//
///**
// * Parses a 'bitwise OR' expression ('|' operator)
// * @param token
// * @return 
// */
//ExprResult parseBitwiseOrExpr(LexToken token)
//{
//    return parseBinaryLROp(token, LEX_TYPES('|'), parseBitwiseXorExpr);
//}
//
///**
// * Parses a 'bitwise XOR' expression ('^' operator)
// * @param token
// * @return 
// */
//ExprResult parseBitwiseXorExpr(LexToken token)
//{
//    return parseBinaryLROp(token, LEX_TYPES('^'), parseBitwiseAndExpr);
//}
//
///**
// * Parses a 'bitwise XOR' expression ('&' operator)
// * @param token
// * @return 
// */
//ExprResult parseBitwiseAndExpr(LexToken token)
//{
//    return parseBinaryLROp(token, LEX_TYPES('&'), parseEqualityExpr);
//}
//
///**
// * Parses a equality expression (see operators in code)
// * @param token
// * @return 
// */
//ExprResult parseEqualityExpr(LexToken token)
//{
//    const int operators[] = {LEX_EQUAL, LEX_TYPEEQUAL, LEX_NEQUAL, LEX_NTYPEEQUAL, 0 };
//    return parseBinaryLROp(token, operators, parseRelationalExpr);
//}
//
///**
// * Parses a relational expression (<, >, <=, >=, instanceof, in) operators
// * @param token
// * @return 
// */
//ExprResult parseRelationalExpr(LexToken token)
//{
//    //TODO: Missing 'instanceof' and 'in' operators
//    const int operators[] = {'<', '>', LEX_LEQUAL, LEX_GEQUAL,  0 };
//    return parseBinaryLROp(token, operators, parseShiftExpr);
//}
//
///**
// * Parses a bit shift expression.
// * @param token
// * @return 
// */
//ExprResult parseShiftExpr(LexToken token)
//{
//    const int operators[] = {LEX_LSHIFT, LEX_RSHIFT, LEX_RSHIFTUNSIGNED,  0 };
//    return parseBinaryLROp(token, operators, parseAddExpr);
//}
//
///**
// * Parses an add or substract expression
// * @param token
// * @return 
// */
//ExprResult parseAddExpr (LexToken token)
//{
//    const int operators[] = {'+', '-', 0 };
//    return parseBinaryLROp(token, operators, parseMultiplyExpr);
//}
//
///**
// * Parses a multiply, divide or modulus expression.
// * @param token
// * @return 
// */
//ExprResult parseMultiplyExpr (LexToken token)
//{
//    const int operators[] = {'*', '/', '%', 0 };
//    return parseBinaryLROp(token, operators, parsePowerExpr);
//}
//
///**
// * Parses a exponenciation operation (operator '**')
// * @param token
// * @return 
// */
//ExprResult parsePowerExpr(LexToken token)
//{
//    return parseBinaryRLOp(token, LEX_POWER, parseUnaryExpr);
//}
//
///**
// * Parses an unary expression
// * @param token
// * @return 
// */
//ExprResult parseUnaryExpr (LexToken token)
//{
//    //TODO: Missing: delete, void & typeof
//    const int operators[] = {'+', '-', '~', '!', LEX_PLUSPLUS, LEX_MINUSMINUS , 0};
//    
//    if (oneOf (token, operators))
//    {
//        ExprResult r = parseUnaryExpr(token.next());
//        
//        if (r.ok())
//        {
//            r.result = astCreatePrefixOp(token, r.result);
//            return r.final();
//        }
//        else
//            return ExprResult(token, r.errorDesc);
//    }
//    else
//        return parsePostFixExpr(token);
//}
//

/**
 * Parses an identifier
 * @param token
 * @return 
 */
ExprResult parseIdentifier (LexToken token)
{
    ExprResult  r = ExprResult(token).require(LEX_ID);
    
    if (r.ok())
        r.result = AstIdentifier::create(token);
    
    return r.final();
}

/**
 * Parses a function call expression
 * @param token
 * @return 
 */
//ExprResult parseCallExpr (LexToken token)
//{
//    ExprResult r = ExprResult(token).then(parseMemberExpr).then(parseCallArguments);
//        
//    while (r.ok() && oneOf (r.token, "([."))
//    {
//        const LEX_TYPES t = r.token.type();
//        
//        if (t == '(')
//            r = r.then(parseCallArguments);
//        else if (t == '[')
//            r = r.then(parseArrayAccess);
//        else
//        {
//            ASSERT (t == '.');
//            r = r.then (parseMemberAccess);
//        }
//    }//while
//    
//    return r.final();
//}

/**
 * Parses a member access expression
 * @note The name is taken from the production of the standard Javascript grammar.
 * But in my opinion, the production has too broad scope to be called just 'member access'
 * @param token
 * @return 
 */
//ExprResult parseMemberExpr (LexToken token)
//{
//    ExprResult r = parsePrimaryExpr(token).orElse(parseFunctionExpr);
//
//    while (!r.error() && oneOf (r.token, "[."))
//    {
//        const LEX_TYPES t = r.token.type();
//
//        if (t == '[')
//            r = r.then(parseArrayAccess);
//        else
//        {
//            ASSERT (t == '.');
//            r = r.then (parseMemberAccess);
//        }
//    }//while
//
//    return r.final();
//}

/**
 * Parses a primary expression.  A primary expression is an identifier name,
 * a constant, an array literal, an object literal or an expression between parenthesis
 * @param token
 * @return 
 */
//ExprResult parsePrimaryExpr (LexToken token)
//{
//    switch ((int)token.type())
//    {
//    case LEX_R_TRUE:    //valueNode = AstLiteral::create(jsTrue()); break;
//    case LEX_R_FALSE:   //valueNode = AstLiteral::create(jsFalse()); break;
//    case LEX_R_NULL:
//    case LEX_FLOAT:
//    case LEX_INT:
//    case LEX_STR:       //valueNode = AstLiteral::create(createConstant(token)); break;
//        return ExprResult (token.next(), AstLiteral::create(token));
//        
//    case LEX_ID:        return parseIdentifier(token);
//    case '[':           return parseArrayLiteral(token);
//    case '{':           return parseObjectLiteral(token);
//    case '(':
//        return ExprResult(token)
//                .require('(')
//                .then(parseExpression)
//                .require(')').final();
//        
//    default:
//        return ExprResult(token)
//                .getError("Unexpected token: '%s'", token.text().c_str());
//    }//switch
//}

/**
 * Parses a function expression
 * @param token
 * @return 
 */

/// <summary>
/// Parses a function definition
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseFunctionDef (LexToken token)
{
    ScriptPosition  pos = token.getPosition();
    string          name;
    ExprResult      r(token);

	r = r.requireReserved("function");
	
	//function name is optional, since unnamed functions are legal.
	if (r.ok() && r.token.type() == LEX_ID)
	{
		name = r.token.text();
		r = r.skip();
	}

	//Parameters tuple.
	r = r.then(parseTupleDef);
	auto params = r.result;

	//return type (optional)
	Ref<AstNode>	returnType;

	if (r.ok() && r.token.isOperator(":"))
	{
		r = r.skip().then(parseIdentifier).orElse(parseTupleDef);
		returnType = r.result;
	}

	r = r.then(parseExpression);

	if (r.ok())
		r.result = astCreateFunction(token.getPosition(), name, params, returnType, r.result);

	return r.final();

    ////unnamed functions are legal.
    //if (r.token.type() == LEX_ID)
    //{
    //    name = r.token.text();
    //    r = r.skip();
    //}
    //
    //Ref<AstFunction>    function = AstFunction::create (pos, name);
    //r.result = function;
    //
    //r = r.then(parseArgumentList).then(parseBlock);

    //if (r.ok())
    //{
    //    function->setCode (r.result);
    //    r.result = function;
    //}
    //
    //return r.final();
}

/// <summary>
/// Parses a primary expression (identifier, constant or parenthesis)
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parsePrimaryExpr(LexToken token)
{
	return parseIdentifier(token)
		.orElse(parseLiteral)
		.orElse(parseParenthesisExpr)
		.orElse(parseTuple)
		.orElse(parseBlock)
		.final();
}


/**
 * Parses an array literal.
 * @param token
 * @return 
 */
//ExprResult parseArrayLiteral (LexToken token)
//{
//    ExprResult     r(token);
//    
//    r = r.require('[');
//    
//    auto    array = astCreateArray(token.getPosition());
//    
//    while (r.ok() && r.token.type() != ']')
//    {
//        //Skip empty entries
//        while (r.token.type() == ',')
//        {
//            array->addChild( AstLiteral::createNull(r.token.getPosition()));
//            r = r.require(',');
//        }
//        
//        if (r.token.type() != ']')
//        {
//            r = r.then(parseAssignment);
//            if (r.ok())
//            {
//                array->addChild(r.result);
//                if (r.token.type() != ']')
//                    r = r.require(',');
//            }
//        }
//    }//while
//    
//    r = r.require(']');
//    if (r.ok())
//        r.result = array;
//    
//    return r.final();
//}

/**
 * Parses an object literal (JSON)
 * @param token
 * @return 
 */
//ExprResult parseObjectLiteral (LexToken token)
//{
//    ExprResult     r(token);
//    
//    r = r.require('{');
//    
//    Ref<AstObject>   object = AstObject::create(token.getPosition());
//    r.result = object;
//    
//    while (r.ok() && r.token.type() != '}')
//    {
//        r = r.then(parseObjectProperty);
//        if (r.token.type() != '}')
//            r = r.require(',');
//    }//while
//    
//    r = r.require('}');
//    
//    return r.final();
//}

/**
 * Parses function call arguments.
 * @note This function creates the function call AST node.
 * @param token     next token
 * @param fnExpr    Expression from which the function reference is obtained
 * @return 
 */
//ExprResult parseCallArguments (LexToken token, Ref<AstNode> fnExpr)
//{
//    ExprResult      r(token);
//
//    r = r.require('(');
//    auto    call = astCreateFnCall(token.getPosition(), fnExpr/*, false*/);
//    
//    while (r.ok() && r.token.type() != ')')
//    {
//        r = r.then(parseAssignment);
//        
//        if (r.ok())
//        {
//            call->addChild(r.result);
//            if (r.token.type() != ')')
//            {
//                r = r.require(',');
//                if (r.token.type() == ')')
//                    r = r.getError("Empty parameter");
//            }
//        }
//    }//while
//    
//    r = r.require(')');
//    if (r.ok())
//        r.result = call;
//    
//    return r.final();
//}

/**
 * Parses an array access expression ('[expression]')
 * @param token
 * @param arrayExpr
 * @return 
 */
//ExprResult parseArrayAccess (LexToken token, Ref<AstNode> arrayExpr)
//{
//    ExprResult     r(token);
//    
//    r = r.require('[').then(parseExpression).require(']');
//    
//    if (r.ok())
//        r.result = astCreateArrayAccess(token.getPosition(), arrayExpr, r.result);
//    
//    return r.final();
//}

/// <summary>
/// Parses a member access expression ('.' operator)
/// </summary>
/// <param name="token"></param>
/// <param name="objExpr"></param>
/// <returns></returns>
ExprResult parseMemberAccess (LexToken token, Ref<AstNode> objExpr)
{
    ExprResult     r(token);
    
    r = r.requireOp(".").then(parseIdentifier);
    
    if (r.ok())
        r.result = astCreateMemberAccess(token.getPosition(), objExpr, r.result);
    
    return r.final();
}

/**
 * Parses a property of an object.
 * @param token
 * @param objExpr
 * @return 
 */
//ExprResult parseObjectProperty (LexToken token, Ref<AstNode> objExpr)
//{
//    string      name;
//    ExprResult  r(token);
//    bool        isConst = false;
//    
//    if (token.type() == LEX_R_CONST)
//    {
//        isConst = true;
//        r = r.skip();
//    }
//    
//    switch (r.token.type())
//    {
//    case LEX_INT:
//    case LEX_FLOAT:
//    case LEX_ID:
//        name = r.token.text();
//        break;
//        
//    case LEX_STR:
//        name = r.token.strValue();
//        break;
//        
//    default:
//        return r.getError("Invalid object property name: %s", r.token.text().c_str()).final();
//    }//switch
//    
//    r = r.skip().require(':').then(parseAssignment);
//    
//    if (r.ok())
//    {
//        objExpr.staticCast<AstObject>()->addProperty (name, r.result, isConst);
//        r.result = objExpr;
//    }
//    
//    return r.final();
//}
//


/**
 * Parses a binary operator which associates from left to right.
 * @param token     startup Token
 * @param opType    Token identifier (from lexer) of the operator
 * @param childParser   Parser for the child expressions.
 * @return 
 */
//ExprResult parseBinaryLROp (LexToken token, LEX_TYPES opType, ExprResult::ParseFunction childParser)
//{
//    const int ids[] = {opType, 0};
//    
//    return parseBinaryLROp (token, ids, childParser);
//}

/**
 * Parses a binary operator which associates from left to right.
 * @param token         startup Token
 * @param ids           array of operator ids. The last element must be zero.
 * @param childParser   Parser for the child expressions.
 * @return 
 */
//ExprResult parseBinaryLROp (LexToken token, const int* ids, ExprResult::ParseFunction childParser)
//{
//    ExprResult      r = childParser (token);
//    
//    while (r.ok() && oneOf (r.token, ids))
//    {
//        const Ref<AstNode>  left = r.result;
//        LexToken        opToken = r.token;
//        
//        r = r.skip();
//        if (r.error())
//            return r.final();
//        
//        r = r.then(childParser);
//            
//        if (r.ok())
//            r.result = astCreateBinaryOp (opToken, left, r.result);
//    }
//
//    return r.final();
//}

///**
// * Parses a binary operator which associates from right to left.
// * @param token     startup Token
// * @param opType    Token identifier (from lexer) of the operator
// * @param childParser   Parser for the child expressions.
// * @return 
// */
//ExprResult parseBinaryRLOp (LexToken token, LEX_TYPES opType, ExprResult::ParseFunction childParser)
//{
//    const int ids[] = {opType, 0};
//    
//    return parseBinaryRLOp (token, ids, childParser);
//}
//
///**
// * Parses a binary operator which associates from right to left.
// * @param token         startup Token
// * @param ids           array of operator ids. The last element must be zero.
// * @param childParser   Parser for the child expressions.
// * @return 
// */
//ExprResult parseBinaryRLOp (LexToken token, const int* ids, ExprResult::ParseFunction childParser)
//{
//    ExprResult      r = childParser (token);
//    
//    if (r.ok() && oneOf (r.token, ids))
//    {
//        const Ref<AstNode>  left = r.result;
//        LexToken        opToken = r.token;
//        
//        r = r.skip();
//        if (r.error())
//            return r.final();
//        
//        r = parseBinaryLROp(r.token, ids, childParser);
//            
//        if (r.ok())
//            r.result = astCreateBinaryOp (opToken, left, r.result);
//        
//        return r.final();
//    }
//    else    
//        return r.final();
//}

/**
 * Parses an actor definition.
 * @param token
 * @return 
 */
ExprResult parseActorExpr (LexToken token)
{
    ScriptPosition  pos = token.getPosition();
    ExprResult      r(token);
    Ref<AstActor>   actorNode;

    r = r.requireReserved("actor");
    if (r.error())
        return r.final();

	//TODO: implement actor parsing
	return r.getError(ETYPE_NOT_IMPLEMENTED_1, "Actor parsing");

    //const string name = r.token.text();
    //
    //r = r.require(LEX_ID);
    //
    //if (r.ok())
    //{
    //    actorNode = AstActor::create(pos, name);
    //    r.result = actorNode;
    //    r = r.then(parseArgumentList).requireOp("{");

    //    while (r.ok() && !r.token.isOperator("}"))
    //    {
    //        //Skip ';', which may (optionally) act as separators.
    //        while (r.token.isOperator (";"))
    //            r = r.skip();
    //        
    //        if (r.token.isOperator("}"))
    //            break;
    //        
    //        r = parseActorMember(r.token);
    //        if (r.ok())
    //            actorNode->addChild(r.result);
    //    }
    //    
    //    r = r.requireOp("}");
    //}//if
    //
    //if (r.ok())
    //    r.result = actorNode;
    //
    //return r.final();
}

///**
// * Parses one of the possible members of an actor.
// * @param token
// * @return 
// */
//ExprResult parseActorMember (LexToken token)
//{
//    switch (token.type())
//    {
//    case LEX_R_VAR:
//    case LEX_R_CONST:
//        return parseDeclaration(token);
//    case LEX_R_INPUT:   return parseInputMessage(token);
//    case LEX_R_OUTPUT:  return parseOutputMessage(token);
//    default:            return parseConnectExpr(token);
//    }
//}
//
///**
// * Parses an input message definition
// * @param token
// * @return 
// */
//ExprResult parseInputMessage (LexToken token)
//{
//    ScriptPosition      pos = token.getPosition();
//    ExprResult          r(token);
//    Ref<AstFunction>    function;
//
//    r = r.require(LEX_R_INPUT);
//    if (r.error())
//        return r.final();
//
//    const string name = r.token.text();
//    r = r.require(LEX_ID);
//
//    if (r.ok())
//    {
//        function = astCreateInputMessage (pos, name);
//        r.result = function;
//    }
//    
//    r = r.then(parseArgumentList).then(parseBlock);
//    
//    if (r.ok())
//    {
//        function->setCode(r.result);
//        r.result = function;
//    }
//    
//    return r.final();
//}
//
///**
// * Parses an output message declaration
// * @param token
// * @return 
// */
//ExprResult parseOutputMessage (LexToken token)
//{
//    ScriptPosition      pos = token.getPosition();
//    ExprResult          r(token);
//
//    r = r.require(LEX_R_OUTPUT);
//    if (r.error())
//        return r.final();
//
//    const string name = r.token.text();
//    r = r.require(LEX_ID);
//
//    if (r.error())
//        return r.final();
//    
//    Ref<AstFunction>    function = astCreateOutputMessage(pos, name);
//    r.result = function;
//    
//    r = r.then(parseArgumentList);
//    
//    if (r.ok())
//        r.result = function;
//    
//    return r.final();
//}
//
///**
// * Parses message connection operator ('<-')
// * @param token
// * @return 
// */
//ExprResult parseConnectExpr (LexToken token)
//{
//    ExprResult  r(token);
//    
//    r = r.then(parseIdentifier);
//    auto lexpr = r.result;
//    
//    ScriptPosition  pos = r.token.getPosition();
//    r = r.require(LEX_CONNECT).then(parseLeftExpr);
//    
//    if (r.ok())
//        r.result = astCreateConnect (pos, lexpr, r.result);    
//    
//    return r.final();
//}
//
//
///**
// * Parses a class definition.
// * @param token
// * @return 
// */
//ExprResult parseClassExpr (LexToken token)
//{
//    ScriptPosition      pos = token.getPosition();
//    ExprResult          r(token);
//    Ref<AstClassNode>   classNode;
//
//    r = r.require(LEX_R_CLASS);
//    if (r.error())
//        return r.final();
//
//    const string name = r.token.text();
//    
//    r = r.require(LEX_ID);
//    
//    if (r.ok())
//    {
//        classNode = AstClassNode::create(pos, name);
//        r.result = classNode;
//        
//        if (r.token.type() == '(')
//            r = r.then(parseArgumentList);
//        
//        if (r.ok() && r.token.type() == LEX_ID && r.token.text() == "extends")
//        {
//            r = r.then(parseExtends);
//            if (r.ok())
//                classNode->addChild(r.result);
//        }
//        
//        r = r.require('{');
//
//        while (r.ok() && r.token.type() != '}')
//        {
//            //Skip ';', which may (optionally) act as separators.
//            while (r.token.type() == ';')
//                r = r.skip();
//            
//            if (r.token.type() == '}')
//                break;
//            
//            r = parseClassMember(r.token);
//            if (r.ok())
//                classNode->addChild(r.result);
//        }
//        
//        r = r.require('}');
//    }//if
//    
//    if (r.ok())
//        r.result = classNode;
//    
//    return r.final();
//}
//
///**
// * Parses a 'extends' clause, used to define class inheritance.
// * @param token
// * @return 
// */
//ExprResult parseExtends (LexToken token)
//{
//    ExprResult  r(token);
//
//    r = r.requireId("extends");
//    if (r.error())
//        return r.final();
//    
//    const string        parentName = r.token.text();
//    r = r.require(LEX_ID);
//    
//    if (r.ok())
//    {
//        auto    extNode = astCreateExtends(token.getPosition(), parentName);
//        
//        if (r.token.type() == '(')
//        {
//            r.result = extNode;
//            r = r.then(parseCallArguments);
//            if (r.ok())
//                extNode->addChild (r.result);
//        }
//        
//        if (r.ok())
//            r.result = extNode;    
//    }
//    
//    
//    return r.final();
//}
//
///**
// * Parses a class member.
// * @param token
// * @return 
// */
//ExprResult parseClassMember (LexToken token)
//{
//    switch (token.type())
//    {
//    case LEX_R_VAR:
//    case LEX_R_CONST:
//        return parseDeclaration(token);
//
//    default:
//        return parseFunctionExpr(token);
//    }
//}
//
///**
// * Parses an 'export' keyword.
// * @param token
// * @return 
// */
//ExprResult parseExport (LexToken token)
//{
//    ExprResult  r(token);
//
//    r = r.require(LEX_R_EXPORT);
//    if (r.error())
//        return r.final();
//    
//    switch ((int)r.token.type())
//    {
//    case LEX_R_VAR:
//    case LEX_R_CONST:
//        r = r.then (parseDeclaration); break;
//
//    case LEX_R_FUNCTION:    r = r.then (parseFunctionExpr); break;
//    case LEX_R_ACTOR:       r = r.then (parseActorExpr); break;
//    case LEX_R_CLASS:       r = r.then (parseClassExpr); break;
//    
//    default:
//        r = r.getError("Unexpected token after 'export': '%s'", token.text().c_str());;
//        break;
//    }//switch
//    
//    if (r.ok())
//        r.result = astCreateExport(token.getPosition(), r.result);
//    
//    return r.final();    
//}
//
///**
// * Parses an import statement
// * @param token
// * @return 
// */
//ExprResult parseImport (LexToken token)
//{
//    ExprResult  r(token);
//
//    r = r.require(LEX_R_IMPORT);
//    if (r.error())
//        return r.final();
//    
//    auto paramToken = r.token;
//    r = r.require(LEX_STR);
//    
//    if (r.ok())
//        r.result = astCreateImport (token.getPosition(), AstLiteral::create(paramToken));
//    
//    return r.final();
//}
