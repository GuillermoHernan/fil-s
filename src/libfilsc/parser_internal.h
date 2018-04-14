/// <summary>
/// Declares functions used internally by the parser.
/// This header is needed for unit testing.
/// </summary>

#pragma once

#include "parser.h"

class LexToken;

ExprResult parseList(LexToken token, ExprResult::ParseFunction parseFn,
    const char* beginTok, const char* endTok, const char* separator);


ExprResult parseTopLevelItem(LexToken token);
ExprResult parseTypedef(LexToken token);
ExprResult parseStruct(LexToken token);
ExprResult parseBlock(LexToken token);
ExprResult  parseDeclaration(LexToken token);
ExprResult  parseAnyDeclaration(LexToken token);
ExprResult  parseArrayDeclaration(LexToken token);
ExprResult  parseConst(LexToken token);
ExprResult  parseVar(LexToken token);
ExprResult  parseTypeSpecifier(LexToken token);
ExprResult  parseTypeDescriptor(LexToken token);
ExprResult  parseTupleDef(LexToken token);
ExprResult  parseTupleDefItem(LexToken token);

ExprResult parseIf(LexToken token);
ExprResult parseSelect(LexToken token);
ExprResult parseReturn(LexToken token);

ExprResult parseExpression(LexToken token);
ExprResult parseAssignment(LexToken token);
ExprResult parseLeftExpr(LexToken token);

ExprResult parseBinaryExpr(LexToken token);
ExprResult parsePrefixExpr(LexToken token);
ExprResult parseTerm(LexToken token);
ExprResult parsePostfixExpr(LexToken token);
ExprResult parsePostfixOperator(LexToken token, Ref<AstNode> termExpr);

ExprResult parseCallExpr(LexToken token, Ref<AstNode> fnExpr);
ExprResult parseLiteral(LexToken token);
ExprResult parseParenthesisExpr(LexToken token);
ExprResult parseTuple(LexToken token);


ExprResult parseConditional(LexToken token);
ExprResult parseIdentifier(LexToken token);

ExprResult parsePrimaryExpr(LexToken token);
ExprResult parseFunctionDef(LexToken token);
ExprResult parseFunctionType(LexToken token);
ExprResult parseInputType(LexToken token);
ExprResult parseMemberAccess(LexToken token, Ref<AstNode> objExpr);

ExprResult parseActorDef(LexToken token);
ExprResult parseInputMsg(LexToken token);
ExprResult parseOutputMsg(LexToken token);
ExprResult parseMsgHeader(LexToken token);
ExprResult parseUnnamedInput(LexToken token);

ExprResult parseImport(LexToken token);
ExprResult parseCMark(LexToken token);

ExprResult parseStatementSeparator(ExprResult prevResult);
bool followsStatementSeparator(ExprResult prevResult);


void addFlagsToChildren(Ref<AstNode> node, int flags);

bool isAssignment(LexToken token);
bool isBinaryOp(LexToken token);
bool isPrefixOp(LexToken token);
bool isPostfixOp(LexToken token);
