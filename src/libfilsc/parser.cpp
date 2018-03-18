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
    auto		result = AstNode::create(AST_LIST, token.getPosition(), "");
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
/// Parses a script, reading it from a file.
/// </summary>
/// <param name="fileRef">File reference object</param>
/// <returns></returns>
ExprResult parseFile(SourceFilePtr fileRef)
{
    std::string content = readTextFile(fileRef->path());

    return parseScript(content.c_str(), fileRef);
}


/// <summary>
/// 
/// </summary>
/// <param name="script"</param>
/// <returns></returns>
/// 

/// <summary>
/// Parses a script.
/// </summary>
/// <param name="script">String which contains the complete source code.</param>
/// <param name="fileRef">File reference, to include in script position objects.</param>
/// <returns></returns>
ExprResult parseScript(const char* script, SourceFilePtr fileRef)
{
    LexToken tok(script, fileRef);

    return parseScript(tok.next());
}


/**
 * Parses an script, which is a list of statements
 * @param token
 * @return
 */
ExprResult parseScript(LexToken token)
{
    //TODO: Set script name
    auto		script = astCreateScript(token.getPosition(), "");

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
        .orElse(parseActorDef)
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
ExprResult parseBlock(LexToken token)
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
ExprResult parseDeclaration(LexToken token)
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
        {
            auto r = parseTypeDescriptor(token);

            if (r.ok())
                r.result = astCreateDeclaration(r.result->position(), "", r.result, Ref<AstNode>());

            return r;
        }
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


/// <summary>
///  Parses a return statement
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseReturn(LexToken token)
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
ExprResult parseExpression(LexToken token)
{
    return parseAssignment(token)
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
ExprResult parseAssignment(LexToken token)
{
    ExprResult r = parseLeftExpr(token);

    auto	lexpr = r.result;
    auto	opToken = r.nextToken();

    r = r.require(isAssignment).then(parseExpression);

    if (r.ok())
        r.result = astCreateAssignment(opToken, lexpr, r.result);

    return r.final();
}

/// <summary>
/// Parses an expression which can be at the left side of an assignment
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseLeftExpr(LexToken token)
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
    auto r = ExprResult::require(isPrefixOp, token).noNewLine().then(parseTerm);

    if (r.ok())
        r.result = astCreatePrefixOp(token, r.result);

    return r.final();
}

/// <summary>
/// Parses a postfix expression
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parsePostfixExpr(LexToken token)
{
    ExprResult	r = parsePrimaryExpr(token);

    if (isPostfixOp(r.nextToken()))
        return r.noNewLine().then(parsePostfixOperator).final();

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
    Ref<AstNode>		value;

    switch ((int)token.type())
    {
    case LEX_RESERVED:
        if (token.text() == "true")
            value = astCreateBool(token.getPosition(), true);
        else if (token.text() == "false")
            value = astCreateBool(token.getPosition(), false);
        break;

    case LEX_FLOAT:
    case LEX_INT:
    case LEX_STR:
        value = astCreateLiteral(token);
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

/// <summary>
/// Parses an identifier
/// </summary>
ExprResult parseIdentifier(LexToken token)
{
    ExprResult  r = ExprResult::require(LEX_ID, token);

    if (r.ok())
    {
        string name = token.text();
        r.result = AstNode::create(AST_IDENTIFIER, token.getPosition(), name, name);
    }

    return r.final();
}

/// <summary>
/// Parses a function definition
/// </summary>
ExprResult parseFunctionDef(LexToken token)
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
        addFlagsToChildren(params, ASTF_FUNCTION_PARAMETER);

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

/// <summary>
/// Parses a member access expression ('.' operator)
/// </summary>
/// <param name="token"></param>
/// <param name="objExpr"></param>
/// <returns></returns>
ExprResult parseMemberAccess(LexToken token, Ref<AstNode> objExpr)
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
ExprResult parseActorDef(LexToken token)
{
    auto	r = ExprResult::requireReserved("actor", token).then(parseIdentifier);

    if (!r.ok())
        return r.final();

    string			name = r.result->getName();
    Ref<AstNode>    actor = astCreateActor(token.getPosition(), name);

    if (r.nextText() == "(")
    {
        r = r.then(parseTupleDef);
        auto params = r.result;

        if (r.ok())
        {
            addFlagsToChildren(params, ASTF_FUNCTION_PARAMETER | ASTF_ACTOR_MEMBER);
            actor->addChild(params);
        }
    }
    else
    {
        //No parameters, add an empty tuple definition.
        actor->addChild(astCreateTupleDef(r.nextToken().getPosition(), ""));
    }

    r = r.requireOp("{");

    while (r.ok() && r.nextText() != "}")
    {
        r = r.then(parseInputMsg)
            .orElse(parseOutputMsg)
            .orElse(parseVar)
            .orElse(parseConst)
            .orElse(parseTypedef)
            .orElse(parseUnnamedInput);

        if (r.ok())
        {
            r.result->addFlag(ASTF_ACTOR_MEMBER);
            actor->addChild(r.result);

            if (r.nextText() != "}")
                r = parseStatementSeparator(r);
        }
    }

    r = r.requireOp("}");

    if (r.ok())
        r.result = actor;

    return r.final();
}

/// <summary>
/// Parses an input message definition.
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseInputMsg(LexToken token)
{
    auto	r = ExprResult::requireReserved("input", token).then(parseMsgHeader);

    auto header = r.result;
    r = r.then(parseBlock);

    if (r.ok())
    {
        auto block = r.result;
        r.result = astCreateInputMsg(token.getPosition(), header->getName());
        r.result->addChild(header->child(0));
        r.result->addChild(block);
    }

    return r;
}


/// <summary>
/// Parses an output message declaration.
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseOutputMsg(LexToken token)
{
    auto	r = ExprResult::requireReserved("output", token).then(parseMsgHeader);

    if (r.ok())
    {
        auto header = r.result;
        r.result = astCreateOutputMsg(token.getPosition(), header->getName());
        r.result->addChild(header->child(0));
    }

    return r;
}

/// <summary>
/// Parses the header (name + parameters) o an input or output message.
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseMsgHeader(LexToken token)
{
    auto	r = parseIdentifier(token);

    if (!r.ok())
        return r.final();

    string name = r.result->getName();

    //Parameters tuple.
    r = r.then(parseTupleDef);

    if (r.ok())
    {
        auto params = r.result;
        r.result = AstNode::create(AST_LIST, token.getPosition(), name);
        addFlagsToChildren(params, ASTF_FUNCTION_PARAMETER);
        r.result->addChild(params);
    }

    return r.final();
}

/// <summary>
/// Parses a 'connect' expression to an unnamed input.
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
ExprResult parseUnnamedInput(LexToken token)
{
    auto r = parseList(token, parseIdentifier, "", "->", ".");
    auto messageRoute = r.result;

    r = r.then(parseTupleDef);
    auto params = r.result;

    r = r.then(parseBlock);
    auto code = r.result;

    if (r.ok())
    {
        //Change to member names, in oder to not be type-checked.
        for (auto pathItem : messageRoute->children())
            pathItem->changeType(AST_MEMBER_NAME);

        addFlagsToChildren(params, ASTF_FUNCTION_PARAMETER);
        r.result = astCreateUnnamedInput(token.getPosition(), messageRoute, params, code);
    }

    return r.final();
}


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
/// Adds the given flag to all children of the node.
/// </summary>
/// <param name="paramsNode"></param>
void addFlagsToChildren(Ref<AstNode> node, int flags)
{
    for (auto child : node->children())
        child->addFlags(flags);
}
