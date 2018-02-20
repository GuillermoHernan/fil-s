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
	auto		result = refFromNew(new AstNamedBranch(AST_LIST, token.getPosition(), ""));
	ExprResult	r = ExprResult::ok(token, result);

	if (*beginTok)
		r = ExprResult::require(beginTok, token);
	else if (token.text() != endTok)
	{
		r = itemParseFn(token);
		result->addChild(r.result);
	}
	else
		return r;

	while (r.ok() && r.nextText() != endTok)
	{
		if (*separator && result->childCount() > 0)
			r = r.requireOp(separator);

		r = r.then(itemParseFn);

		result->addChild(r.result);
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
    auto		script = astCreateScript(token.getPosition());

	if (token.eof())
		return ExprResult::ok(token, script);

	ExprResult	r = parseTopLevelItem(token);

	script->addChild(r.result);

    while (r.ok() && r.nextType() != LEX_EOF)
    {
		r = parseStatementSeparator(r);
		if (r.ok() && r.nextType() != LEX_EOF)
		{
			r = r.then(parseTopLevelItem);
			script->addChild(r.result);
		}
    }

	if (r.ok())
		r.result = script;

    return r;
}

/// <summary>
/// Parses a top-level item. An item which is not part of another language construct.
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseTopLevelItem(LexToken token)
{
	return parseConst(token)
		.orElse(parseActorExpr)
		.orElse(parseFunctionDef)
		.orElse(parseTypedef);
}

/// <summary>
/// Parses a type definition.
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseTypedef(LexToken token)
{
	string		name;

	ExprResult r = ExprResult::requireReserved("type", token).then(parseIdentifier);

	if (r.ok())
		name = r.result->getName();

	r = r.requireId("is").then(parseTypeDescriptor);

	if (r.ok())
		r.result = astCreateTypedef(token.getPosition(), name, r.result);

	return r.final();
}


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
    ExprResult      r = ExprResult::require("{", token);
    
    while (r.ok() && r.nextText() != "}")
    {
		r = r.then(parseReturn)
			.orElse(parseVar)
			.orElse(parseConst)
			.orElse(parseTypedef);

		if (r.ok())
		{
			block->addChild(r.result);

			if (r.nextText() != "}")
				r = parseStatementSeparator(r);
		}
    }
    
    r = r.requireOp("}");
    
    if (r.ok())
        r.result = block;
    
    return r.final();
}

/// <summary>
/// Parses a declaration. It does not parse the modifiers. They should be parsed
/// by the calling function.
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseDeclaration (LexToken token)
{
	Ref<AstNode>	typeDescriptor;
	Ref<AstNode>	initExp;

	auto r = ExprResult::require(LEX_ID, token);
	if (!r.ok())
		return r.final();

	//Type descriptor is optional.
	if (r.nextText() == ":")
	{
		r = r.then(parseTypeSpecifier);
		typeDescriptor = r.result;
	}

	//Initialization is also optional.
	if (r.nextText() == "=")
	{
		r = r.skip().then(parseExpression);

		initExp = r.result;
	}

	if (r.ok())
		r.result = astCreateDeclaration(token, typeDescriptor, initExp);

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
	auto r = ExprResult::requireReserved("const", token).then(parseDeclaration);

	if (r.ok())
		r.result->addFlag(ASTF_CONST);

	return r.final();
}

/// <summary>
/// Parses a variable definition expression
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseVar(LexToken token)
{
	auto r = ExprResult::requireReserved("var", token).then(parseDeclaration);

	if (r.ok())
		r.result->addFlag(ASTF_VAR);

	return r.final();
}

/// <summary>
/// Parses a type specifier for a declaration.
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseTypeSpecifier(LexToken token)
{
	return ExprResult::require(":", token).then(parseTypeDescriptor);
}

/// <summary>
/// Parses a type descriptor, which can be just a type name or a tuple/structure
/// declaration.
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseTypeDescriptor(LexToken token)
{
	ExprResult r = parseIdentifier(token).orElse(parseTupleDef);

	if (r.ok() && r.result->getType() == AST_IDENTIFIER)
		r.result->changeType(AST_TYPE_NAME);

	return r;
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
	if (token.type() == LEX_ID)
	{
		LexToken next = token.next();

		if (next.type() == LEX_OPERATOR && (next.text() == ":" || next.text() == "="))
			return parseAnyDeclaration(token);
		else
			return parseTypeDescriptor(token);
	}
	else
		return parseAnyDeclaration(token).orElse(parseTypeDescriptor);
}

/// <summary>
/// Parses an 'if' flow control expression.
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseIf(LexToken token)
{
	auto r = ExprResult::requireReserved("if", token).requireOp("(").then(parseExpression);

	auto conditionExpr = r.result;

	r = r.requireOp(")").then(parseReturn);

	auto			thenExpr = r.result;

	//A single semicolon may follow 'then' expression.
	if (r.ok() && r.nextText() == ";")
		r = r.skip();

	Ref<AstNode>	elseExpr;

	//Check for the presence of 'else'
	if (r.ok() && r.nextText() == "else")
	{
		r = r.requireReserved("else").then(parseReturn);
		elseExpr = r.result;
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
	auto r = ExprResult::requireReserved("select", token);

	if (r.ok())
		r = r.getError(ETYPE_NOT_IMPLEMENTED_1, "'select' parsing");

	return r.final();
}


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

/// <summary>
///  Parses a return statement
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseReturn (LexToken token)
{
	auto r = ExprResult::requireReserved("return", token);

	//If it is not a return, fallback to parse an expression.
	if (!r.ok())
		return parseExpression(token);
	else
	{
		if (followsStatementSeparator(r))
		{
			r.result = astCreateReturn(token.getPosition(), Ref<AstNode>());
			return r;
		}
		else
		{
			r = r.then(parseExpression);
			if (r.ok())
				r.result = astCreateReturn(token.getPosition(), r.result);
		}
	}

	return r.final();
}

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
	auto	opToken = r.nextToken();

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
	auto		opToken = r.nextToken();

	r = r.require(isBinaryOp).then(parseTerm);
	if (r.ok())
	{
		auto result = astCreateBinaryOp(opToken, leftExpr, r.result);

		//Parse chained operations.
		while (r.ok() && r.nextText() == opToken.text())
		{
			opToken = r.nextToken();
			r = r.skip().then(parseTerm);

			if (r.ok())
				result = astCreateBinaryOp(opToken, result, r.result);
		}

		if (r.ok())
		{
			if (isBinaryOp(r.nextToken()))
				r = r.skip().getError(ETYPE_INVALID_EXP_CHAIN);
			else
				r.result = result;
		}
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
	auto r = ExprResult::require(isPrefixOp, token).then(parseTerm);

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

	if (isPostfixOp(r.nextToken()))
		return r.then(parsePostfixOperator).final();

	while (r.ok())
	{
		string	opText = r.nextText(LexToken::NEWLINE);
		bool	newLine = false;

		if (opText == "\n")
		{
			newLine = true;
			opText = r.nextText();
		}

		if (opText == ".")
			r = r.then(parseMemberAccess);
		else if (opText == "(" && !newLine)
			r = r.then(parseCallExpr);
		else
			break;
	}

	return r.final();
}

/// <summary>
/// Parses a postfix operator
/// </summary>
/// <param name="token"></param>
/// <param name="termExpr"></param>
/// <returns></returns>
ExprResult parsePostfixOperator(LexToken token, Ref<AstNode> termExpr)
{
	auto r = ExprResult::require(isPostfixOp, token);
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
	case LEX_RESERVED:
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
		return ExprResult::ok(token, value);
	else
		return ExprResult::getError(token, ETYPE_UNEXPECTED_TOKEN_2, token.text().c_str(), "literal");
}


/// <summary>
/// Parses a expression between parenthesis '(...)'.
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseParenthesisExpr(LexToken token)
{
	return ExprResult::require("(", token)
		.then(parseExpression)
		.requireOp(")")
		.final();
}

/// <summary>
/// Parses a tuple creation '(a,b,c...)'
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseTuple(LexToken token)
{
	Ref<AstNode>	result = astCreateTuple(token);
	auto			r = ExprResult::require("(", token);

	if (r.nextText() != ")")
	{
		r = r.then(parseExpression);
		result->addChild(r.result);

		while (r.ok() && r.nextText() != ")")
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
 * Parses an identifier
 * @param token
 * @return 
 */
ExprResult parseIdentifier (LexToken token)
{
    ExprResult  r = ExprResult::require(LEX_ID, token);
    
    if (r.ok())
        r.result = AstIdentifier::create(token);
    
    return r.final();
}

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
    auto			r = ExprResult::requireReserved("function", token);

	//function name is optional, since unnamed functions are legal.
	if (r.ok() && r.nextType() == LEX_ID)
	{
		name = r.nextText();
		r = r.skip();
	}

	//Parameters tuple.
	r = r.then(parseTupleDef);
	auto params = r.result;

	if (r.ok())
		markAsParameters(params);

	//return type (optional)
	Ref<AstNode>	returnType;

	if (r.ok() && r.nextText() == ":")
	{
		r = r.skip().then(parseTypeDescriptor);
		returnType = r.result;
	}

	r = r.then(parseExpression);

	if (r.ok())
		r.result = astCreateFunction(token.getPosition(), name, params, returnType, r.result);

	return r.final();
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
    auto r = ExprResult::require(".", token).then(parseIdentifier);
    
	if (r.ok())
	{
		auto memberNameNode = r.result;

		memberNameNode->changeType(AST_MEMBER_NAME);
		r.result = astCreateMemberAccess(token.getPosition(), objExpr, memberNameNode);
	}
    
    return r.final();
}

/**
 * Parses an actor definition.
 * @param token
 * @return 
 */
ExprResult parseActorExpr (LexToken token)
{
    Ref<AstActor>   actorNode;

    auto r = ExprResult::requireReserved("actor", token);
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

/// <summary>
/// Parses one or several statement separators.
/// </summary>
/// <param name="r">Previous result</param>
/// <returns></returns>
ExprResult parseStatementSeparator(ExprResult r)
{
	if (!r.ok())
		return r;

	bool		found = false;
	LexToken	next = r.nextToken(LexToken::NEWLINE);

	while (next.type() == LEX_NEWLINE || next.text() == ";")
	{
		found = true;
		r = ExprResult::ok(next, r.result);
		next = r.nextToken(LexToken::NEWLINE);
	}

	if (found)
		return r;
	else
		return r.skip().getError(ETYPE_UNEXPECTED_TOKEN_2, 
			next.text().c_str(), 
			"statement separator (';' or new line)");
}

/// <summary>
/// Checks if a statement separator is the next token.
/// </summary>
bool followsStatementSeparator(ExprResult r)
{
	if (!r.ok())
		return false;

	LexToken	next = r.nextToken(LexToken::NEWLINE);

	return (next.type() == LEX_NEWLINE || next.text() == ";");
}



/// <summary>
/// Marks all children of 'node' as function parameters.
/// </summary>
/// <param name="paramsNode"></param>
void markAsParameters(Ref<AstNode> node)
{
	for (auto child : node->children())
		child->addFlag(ASTF_FUNCTION_PARAMETER);
}
