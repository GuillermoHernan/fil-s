/// <summary>
/// Declares functions used internally by the parser.
/// This header is needed for unit testing.
/// </summary>

#pragma once

#include "parser.h"

class LexToken;

//Ref<AstNode>   emptyStatement(ScriptPosition pos);

ExprResult parseList(LexToken token, ExprResult::ParseFunction parseFn,
	const char* beginTok, const char* endTok, const char* separator);


ExprResult parseTopLevelItem(LexToken token);
//ExprResult parseSimpleStatement(LexToken token);
//ExprResult parseBodyStatement(LexToken token);
ExprResult parseTypedef(LexToken token);
ExprResult parseBlock(LexToken token);
ExprResult  parseDeclaration(LexToken token);
ExprResult  parseAnyDeclaration(LexToken token);
ExprResult  parseConst(LexToken token);
ExprResult  parseVar(LexToken token);
ExprResult  parseTypeSpecifier(LexToken token);
ExprResult  parseTypeDescriptor(LexToken token);
ExprResult  parseTupleDef(LexToken token);
ExprResult  parseTupleDefItem(LexToken token);

ExprResult parseIf(LexToken token);
ExprResult parseSelect(LexToken token);
//ExprResult parseWhile(LexToken token);
//ExprResult parseFor(LexToken token);
//ExprResult parseForEach(LexToken token);
ExprResult parseReturn(LexToken token);
//ExprResult parseArgumentList(LexToken token, Ref<AstNode> function);
//ExprResult parseStructDef(LexToken token);

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
//ExprResult parseLogicalOrExpr(LexToken token);
//ExprResult parseLogicalAndExpr(LexToken token);
//ExprResult parseBitwiseOrExpr(LexToken token);
//ExprResult parseBitwiseXorExpr(LexToken token);
//ExprResult parseBitwiseAndExpr(LexToken token);
//ExprResult parseEqualityExpr(LexToken token);
//ExprResult parseRelationalExpr(LexToken token);
//ExprResult parseShiftExpr(LexToken token);
//ExprResult parseAddExpr(LexToken token);
//ExprResult parseMultiplyExpr(LexToken token);
//ExprResult parsePowerExpr(LexToken token);
//ExprResult parseUnaryExpr(LexToken token);
ExprResult parseIdentifier(LexToken token);

//ExprResult parseNewExpr (LexToken token);
//ExprResult parseMemberExpr(LexToken token);
ExprResult parsePrimaryExpr(LexToken token);
ExprResult parseFunctionDef(LexToken token);
//ExprResult parseArrayLiteral(LexToken token);
//ExprResult parseObjectLiteral(LexToken token);
//ExprResult parseCallArguments(LexToken token, Ref<AstNode> fnExpr);
//ExprResult parseArrayAccess(LexToken token, Ref<AstNode> arrayExpr);
ExprResult parseMemberAccess(LexToken token, Ref<AstNode> objExpr);
//ExprResult parseObjectProperty(LexToken token, Ref<AstNode> objExpr);

//ExprResult parseBinaryLROp(LexToken token, LEX_TYPES opType, ExprResult::ParseFunction childParser);
//ExprResult parseBinaryLROp(LexToken token, const int *types, ExprResult::ParseFunction childParser);
//ExprResult parseBinaryRLOp(LexToken token, LEX_TYPES opType, ExprResult::ParseFunction childParser);
//ExprResult parseBinaryRLOp(LexToken token, const int *types, ExprResult::ParseFunction childParser);

ExprResult parseActorExpr(LexToken token);
//ExprResult parseActorMember(LexToken token);
//ExprResult parseInputMessage(LexToken token);
//ExprResult parseOutputMessage(LexToken token);
//ExprResult parseConnectExpr(LexToken token);
//
//ExprResult parseClassExpr(LexToken token);
//ExprResult parseExtends(LexToken token);
//ExprResult parseClassMember(LexToken token);
//
//ExprResult parseExport(LexToken token);
//ExprResult parseImport(LexToken token);

ExprResult parseStatementSeparator(ExprResult prevResult);

void markAsParameters(Ref<AstNode> node);

bool isAssignment(LexToken token);
bool isBinaryOp(LexToken token);
bool isPrefixOp(LexToken token);
bool isPostfixOp(LexToken token);
