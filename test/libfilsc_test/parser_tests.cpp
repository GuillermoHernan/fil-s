/// <summary>
/// Tests FIL-S parser.
/// </summary>

#include "libfilsc_test_pch.h"
#include "parser_internal.h"
#include "compileError.h"

/// <summary>
/// Tests 'parseScript' function.
/// </summary>
TEST(Parser, parseScript)
{
	const char* goodCode = "function add (a:int, b:int) {a+b}\n";

	const char* badCode = "funcion add (a:int, b:int) {a+b}\n";

	auto result = parseScript(goodCode);
	EXPECT_STREQ("", result.errorDesc.what());
	ASSERT_TRUE(result.ok());
	EXPECT_EQ(AST_SCRIPT, result.result->getType());

	result = parseScript(badCode);
	EXPECT_FALSE(result.ok());

	result = parseScript("");
	EXPECT_TRUE(result.ok());
	EXPECT_FALSE(result.result->childExists(0));
}

/// <summary>
/// Tests 'isAssignment' function, which checks if an operator is an assignment operator.
/// </summary>
TEST(Parser, isAssignment)
{
	const char * assignmentOps = "= >>>= >>= <<= **= += -= *= /= %= &= |= ^=";
	const char * otherOps = ">>> >> << ** + - * / % < > <= >= ";

	LexToken	tok(assignmentOps);

	for (tok = tok.next(); !tok.eof(); tok = tok.next())
		EXPECT_TRUE(isAssignment(tok));

	tok = LexToken(otherOps);

	for (tok = tok.next(); !tok.eof(); tok = tok.next())
		EXPECT_FALSE(isAssignment(tok));
}


/// <summary>
/// Tests 'isBinaryOp' function, which checks if an operator is a binary operator.
/// </summary>
TEST(Parser, isBinaryOp)
{
	const char * binaryOps = ">>> >> << ** + - * / % & | && || ^ < > >= <= == != ";
	const char * otherOps = "-= += ~ ! ++ --";

	LexToken	tok(binaryOps);

	for (tok = tok.next(); !tok.eof(); tok = tok.next())
		EXPECT_TRUE(isBinaryOp(tok));

	tok = LexToken(otherOps);

	for (tok = tok.next(); !tok.eof(); tok = tok.next())
		EXPECT_FALSE(isBinaryOp(tok));
}


/// <summary>
/// Tests 'isPrefixOp' function, which checks if an operator is a prefix operator.
/// </summary>
TEST(Parser, isPrefixOp)
{
	const char * prefixOps = "! ~ + - ++ --";
	const char * otherOps = ">>> >> << ** * / % & | && || ^ < > >= <= == != ";

	LexToken	tok(prefixOps);

	for (tok = tok.next(); !tok.eof(); tok = tok.next())
		EXPECT_TRUE(isPrefixOp(tok));

	tok = LexToken(otherOps);

	for (tok = tok.next(); !tok.eof(); tok = tok.next())
		EXPECT_FALSE(isPrefixOp(tok));
}


/// <summary>
/// Tests 'isPostfixOp' function, which checks if an operator is a postfix operator.
/// </summary>
TEST(Parser, isPostfixOp)
{
	const char * postFixOps = "++ --";
	const char * otherOps = "! | ~ >>> >> << ** * / % & | && || ^ < > >= <= == != ";

	LexToken	tok(postFixOps);

	for (tok = tok.next(); !tok.eof(); tok = tok.next())
		EXPECT_TRUE(isPostfixOp(tok));

	tok = LexToken(otherOps);

	for (tok = tok.next(); !tok.eof(); tok = tok.next())
		EXPECT_FALSE(isPostfixOp(tok));
}

/// <summary>
/// Tests for 'parseList' function. This function is used to parse nodes which are build from 
/// a list of other nodes.
/// </summary>
TEST(Parser, parseList)
{
	const char * fullList = "(1,2,3,4,5,6,7)";
	const char * sepList = "1,2,3,4,5,6,7)";
	const char * nosepList = "{1 2 3 4 5 6 7}";
	const char * simpleList = "1 2 3 4 5 6 7)";

	auto parseList_ = [](const char* code, const char* beginTok, const char* endTok, const char* separator)
	{
		LexToken	tok(code);
		return parseList(tok.next(), parseLiteral, beginTok, endTok, separator);
	};

	EXPECT_PARSE_OK(parseList_(fullList, "(", ")", ","));
	EXPECT_PARSE_ERROR(parseList_(fullList, "", ")", ","));
	EXPECT_PARSE_ERROR(parseList_(fullList, "", ")", ""));
	EXPECT_PARSE_ERROR(parseList_(fullList, "(", ")", ""));

	EXPECT_PARSE_ERROR(parseList_(sepList, "(", ")", ","));
	EXPECT_PARSE_OK(parseList_(sepList, "", ")", ","));
	EXPECT_PARSE_ERROR(parseList_(sepList, "", ")", ""));
	EXPECT_PARSE_ERROR(parseList_(sepList, "(", ")", ""));

	EXPECT_PARSE_ERROR(parseList_(nosepList, "{", "}", ","));
	EXPECT_PARSE_ERROR(parseList_(nosepList, "", "}", ","));
	EXPECT_PARSE_ERROR(parseList_(nosepList, "", "}", ""));
	EXPECT_PARSE_OK(parseList_(nosepList, "{", "}", ""));
	EXPECT_PARSE_ERROR(parseList_(nosepList, "(", ")", ""));

	EXPECT_PARSE_ERROR(parseList_(simpleList, "(", "#", ","));
	EXPECT_PARSE_ERROR(parseList_(simpleList, "", ")", ","));
	EXPECT_PARSE_OK(parseList_(simpleList, "", ")", ""));
	EXPECT_PARSE_ERROR(parseList_(simpleList, "(", ")", ""));
}

/// <summary>
/// Tests for 'parseBlock' function.
/// </summary>
/// <param name=""></param>
/// <param name=""></param>
/// <returns></returns>
TEST(Parser, parseBlock)
{
	auto parseBlock_ = [](const char* code)
	{
		return parseBlock(LexToken(code).next());
	};

	const char * block1 =
		"{\n"
		"  const a = 7 + 6\n"
		"  const b = 4 * 5\n"
		"  b - a\n"
		"}\n";

	const char * withSemicolons =
		"{\n"
		"  const a = 7 + 6;\n"
		"  const b = 4 * 5;;;\n"
		"  b - a;\n"
		"}\n";

	const char * errorInExp = "{10 - 4 , 3}";
	const char * notClosed = "{10 + 4 + 3";
	const char * wrongCloseToken = "{10 + 4 + 3)";
	const char * notBlock = "(10 + 4 + 3)";

	EXPECT_PARSE_OK(parseBlock_(block1));
	EXPECT_PARSE_OK(parseBlock_(withSemicolons));

	EXPECT_PARSE_ERROR(parseBlock_(errorInExp));
	EXPECT_PARSE_ERROR(parseBlock_("{%n}"));
	EXPECT_PARSE_ERROR(parseBlock_(notClosed));
	EXPECT_PARSE_ERROR(parseBlock_(wrongCloseToken));
	EXPECT_PARSE_ERROR(parseBlock_(notBlock));
}

/// <summary>
/// Tests 'parseDeclaration' function.
/// Also tests 'parseTypeSpecifier' and 'parseTypeDescriptor' functions.
/// </summary>
TEST(Parser, parseDeclaration)
{
	auto parseDeclaration_ = [](const char* code)
	{
		return checkAllParsed(code, parseDeclaration);
	};

	EXPECT_PARSE_OK(parseDeclaration_("standAlone"));
	EXPECT_PARSE_OK(parseDeclaration_("typed:int"));
	EXPECT_PARSE_OK(parseDeclaration_("initialized = a"));
	EXPECT_PARSE_OK(parseDeclaration_("full:int = 3"));
	EXPECT_PARSE_OK(parseDeclaration_("tuple1:(int, int, float)"));

	EXPECT_PARSE_ERROR(parseDeclaration_("var full:int = a"));
	EXPECT_PARSE_ERROR(parseDeclaration_("if = 3"));
	EXPECT_PARSE_ERROR(parseDeclaration_("const: int"));

}

/// <summary>
/// Tests 'parseConst' and 'parseVar' functions.
/// </summary>
TEST(Parser, parseVarConst)
{
	auto parseVar_ = [](const char* code)
	{
		return checkAllParsed(code, parseVar);
	};
	auto parseConst_ = [](const char* code)
	{
		return checkAllParsed(code, parseConst);
	};

	EXPECT_PARSE_OK(parseVar_("var a"));
	EXPECT_PARSE_OK(parseVar_("var a: int = 7"));
	EXPECT_PARSE_OK(parseConst_("const a"));
	EXPECT_PARSE_OK(parseConst_("const a: int = 7"));

	EXPECT_PARSE_ERROR(parseConst_("var a"));
	EXPECT_PARSE_ERROR(parseConst_("var a: int = 7"));
	EXPECT_PARSE_ERROR(parseVar_("const a"));
	EXPECT_PARSE_ERROR(parseVar_("const a: int = 7"));

	EXPECT_PARSE_ERROR(parseVar_("a: int = 7"));
	EXPECT_PARSE_ERROR(parseConst_("a: int = 7"));
}

/// <summary>
/// Tests 'parseTupleDef' and 'parseTupleDefItem' functions
/// </summary>
TEST(Parser, parseTupleDef)
{
	auto parseTupleDef_ = [](const char* code)
	{
		return checkAllParsed(code, parseTupleDef);
	};

	EXPECT_PARSE_OK(parseTupleDef_("(int, int, int)"));
	EXPECT_PARSE_OK(parseTupleDef_("(float)"));
	EXPECT_PARSE_OK(parseTupleDef_("()"));
	EXPECT_PARSE_OK(parseTupleDef_("(a:int, b:float)"));
	EXPECT_PARSE_OK(parseTupleDef_("(const a:int, b:float, var c:string)"));
	EXPECT_PARSE_OK(parseTupleDef_("(int, int, (float, otherType))"));

	EXPECT_PARSE_ERROR(parseTupleDef_("(1, 2, 3)"));
	EXPECT_PARSE_ERROR(parseTupleDef_("(int, int"));
	EXPECT_PARSE_ERROR(parseTupleDef_("u(int, int)"));
	EXPECT_PARSE_ERROR(parseTupleDef_("(int, int; int"));
}


/// <summary>
/// Tests 'parseIf' function
/// </summary>
TEST(Parser, parseIf)
{
	auto parseIf_ = [](const char* code)
	{
		return checkAllParsed(code, parseIf);
	};

	EXPECT_PARSE_OK(parseIf_("if (a) b"));
	EXPECT_PARSE_OK(parseIf_("if (a) b;"));
	EXPECT_PARSE_OK(parseIf_("if (a) b else c"));
	EXPECT_PARSE_OK(parseIf_("if (a) b; else c"));
	EXPECT_PARSE_OK(parseIf_("if (a) {b; c; d;} else x"));

	EXPECT_PARSE_ERROR(parseIf_("isf (a) b"));
	EXPECT_PARSE_ERROR(parseIf_("if a) b"));
	EXPECT_PARSE_ERROR(parseIf_("if (a b"));
	EXPECT_PARSE_ERROR(parseIf_("if (a b else c"));
	EXPECT_PARSE_ERROR(parseIf_("if (a) b c"));
	EXPECT_PARSE_ERROR(parseIf_("if (a) b;; else c"));
}


/// <summary>
/// Tests 'parseExpression' function
/// </summary>
TEST(Parser, parseExpression)
{
	auto parseExpression_ = [](const char* code)
	{
		return checkAllParsed(code, parseExpression);
	};

	EXPECT_PARSE_OK(parseExpression_("a*=b"));
	EXPECT_PARSE_OK(parseExpression_("a+b"));
	EXPECT_PARSE_OK(parseExpression_("-a"));
	EXPECT_PARSE_OK(parseExpression_("a++"));
	EXPECT_PARSE_OK(parseExpression_("(hello)"));
	EXPECT_PARSE_OK(parseExpression_("if (a) b else c"));

	EXPECT_PARSE_ERROR(parseExpression_("const a = 7"));
	EXPECT_PARSE_ERROR(parseExpression_("var a = 7"));
}

/// <summary>
/// Tests 'parseTerm' function
/// </summary>
TEST(Parser, parseTerm)
{
	auto parseTerm_ = [](const char* code)
	{
		return checkAllParsed(code, parseTerm);
	};

	EXPECT_PARSE_OK(parseTerm_("a"));
	EXPECT_PARSE_OK(parseTerm_("8"));
	EXPECT_PARSE_OK(parseTerm_("\"test\""));
	EXPECT_PARSE_OK(parseTerm_("a.b"));
	EXPECT_PARSE_OK(parseTerm_("(1,2,3)"));
	EXPECT_PARSE_OK(parseTerm_("(a-b)"));
	EXPECT_PARSE_OK(parseTerm_("{a-b}"));
	EXPECT_PARSE_OK(parseTerm_("if (a) b else c"));
	EXPECT_PARSE_OK(parseTerm_("a++"));

	EXPECT_PARSE_ERROR(parseTerm_("a=b"));
	EXPECT_PARSE_ERROR(parseTerm_("a*=b"));
	EXPECT_PARSE_ERROR(parseTerm_("a+b"));
	EXPECT_PARSE_ERROR(parseTerm_("-a"));
}

/// <summary>
/// Tests 'parseAssignment' function
/// </summary>
TEST(Parser, parseAssignment)
{
	auto parseAssignment_ = [](const char* code)
	{
		return checkAllParsed(code, parseAssignment);
	};

	EXPECT_PARSE_OK(parseAssignment_("a = b"));
	EXPECT_PARSE_OK(parseAssignment_("x *= 9"));
	EXPECT_PARSE_OK(parseAssignment_("r %= 9"));

	EXPECT_PARSE_ERROR(parseAssignment_("r %= 9 c"));
	EXPECT_PARSE_ERROR(parseAssignment_("x"));
	EXPECT_PARSE_ERROR(parseAssignment_("x + y"));

	auto result = parseAssignment_("a = b");
	EXPECT_EQ(AST_ASSIGNMENT, result.result->getType());
}

/// <summary>
/// Tests for 'parseLeftExpr' function
/// </summary>
TEST(Parser, parseLeftExpr)
{
	auto parseLeftExpr_ = [](const char* code)
	{
		return checkAllParsed(code, parseLeftExpr);
	};

	EXPECT_PARSE_OK(parseLeftExpr_("a++"));
	EXPECT_PARSE_OK(parseLeftExpr_("a.c"));
	//EXPECT_PARSE_OK(parseLeftExpr_("a.c(1,2)"));
	EXPECT_PARSE_OK(parseLeftExpr_("fn(7)"));
	EXPECT_PARSE_OK(parseLeftExpr_("x"));
	EXPECT_PARSE_OK(parseLeftExpr_("2"));
	EXPECT_PARSE_OK(parseLeftExpr_("(x+6)"));

	EXPECT_PARSE_ERROR(parseLeftExpr_("r = 9"));
	EXPECT_PARSE_ERROR(parseLeftExpr_("3 + 3"));
}

/// <summary>
/// Tests for 'parseBinaryExpr' function
/// </summary>
TEST(Parser, parseBinaryExpr)
{
	auto parseBinaryExpr_ = [](const char* code)
	{
		return checkAllParsed(code, parseBinaryExpr);
	};

	EXPECT_PARSE_OK(parseBinaryExpr_("a + 9"));
	EXPECT_PARSE_OK(parseBinaryExpr_("a + 9 + 7"));
	EXPECT_PARSE_OK(parseBinaryExpr_("a + b.c + 7"));
	EXPECT_PARSE_OK(parseBinaryExpr_("a + 9 + (7-x)"));

	EXPECT_PARSE_ERROR(parseBinaryExpr_("r = 9"));
	EXPECT_PARSE_ERROR(parseBinaryExpr_("-n"));
	EXPECT_PARSE_ERROR(parseBinaryExpr_("a + b - c"));

	auto result = parseBinaryExpr_("a + b - c");
	EXPECT_EQ(ETYPE_INVALID_EXP_CHAIN, result.errorDesc.type());
	EXPECT_EQ(7, result.errorDesc.position().column);
}

/// <summary>
/// Tests for 'parsePrefixExpr' function
/// </summary>
TEST(Parser, parsePrefixExpr)
{
	auto parsePrefixExpr_ = [](const char* code)
	{
		return checkAllParsed(code, parsePrefixExpr);
	};

	EXPECT_PARSE_OK(parsePrefixExpr_("-1"));
	EXPECT_PARSE_OK(parsePrefixExpr_("-a"));
	EXPECT_PARSE_OK(parsePrefixExpr_("+u"));
	EXPECT_PARSE_OK(parsePrefixExpr_("!a"));
	EXPECT_PARSE_OK(parsePrefixExpr_("~r"));
	EXPECT_PARSE_OK(parsePrefixExpr_("++i"));
	EXPECT_PARSE_OK(parsePrefixExpr_("--j"));

	EXPECT_PARSE_ERROR(parsePrefixExpr_("-~c"));
	EXPECT_PARSE_ERROR(parsePrefixExpr_("r + 9"));
	EXPECT_PARSE_ERROR(parsePrefixExpr_("i++"));
}

/// <summary>
/// Tests for 'parsePostfixExpr', 'parsePostfixOperator' and 'parseMemberAccess' functions
/// </summary>
TEST(Parser, parsePostfixExpr)
{
	auto parsePostfixExpr_ = [](const char* code)
	{
		return checkAllParsed(code, parsePostfixExpr);
	};

	EXPECT_PARSE_OK(parsePostfixExpr_("i++"));
	EXPECT_PARSE_OK(parsePostfixExpr_("u--"));
	EXPECT_PARSE_OK(parsePostfixExpr_("a.b"));
	EXPECT_PARSE_OK(parsePostfixExpr_("a.b.c.d.e"));
	EXPECT_PARSE_OK(parsePostfixExpr_("a(3,n)"));
	EXPECT_PARSE_OK(parsePostfixExpr_("a.b(3).c.d(9,(7,a)).e"));

	EXPECT_PARSE_ERROR(parsePostfixExpr_("--c"));
	EXPECT_PARSE_ERROR(parsePostfixExpr_("i++ ++"));
	EXPECT_PARSE_ERROR(parsePostfixExpr_("c+4"));
}

/// <summary>
/// Tests for 'parseCallExpr' function
/// </summary>
TEST(Parser, parseCallExpr)
{
	auto parseCallExpr_ = [](const char* code)
	{
		//It does not call 'parseCallExpr' because it needs the previous expression
		//(the expression which yields the function to be called)
		return checkAllParsed(code, parsePostfixExpr);
	};

	EXPECT_PARSE_OK(parseCallExpr_("fn(b,c)"));
	EXPECT_PARSE_OK(parseCallExpr_("fn(1)"));
	EXPECT_PARSE_OK(parseCallExpr_("fn()"));
	EXPECT_PARSE_OK(parseCallExpr_("obj.fn(1,2)"));
	EXPECT_PARSE_OK(parseCallExpr_("obj.fn(1,2)(3)(4)"));

	EXPECT_PARSE_ERROR(parseCallExpr_("fn 3"));
	EXPECT_PARSE_ERROR(parseCallExpr_("fn (3"));
	EXPECT_PARSE_ERROR(parseCallExpr_("fn 3)"));
}

/// <summary>
/// Tests for 'parseLiteral' function
/// </summary>
TEST(Parser, parseLiteral)
{
	auto parseLiteral_ = [](const char* code)
	{
		return checkAllParsed(code, parseLiteral);
	};

	EXPECT_PARSE_OK(parseLiteral_("1"));
	EXPECT_PARSE_OK(parseLiteral_("1000000"));
	EXPECT_PARSE_OK(parseLiteral_("46.9"));
	EXPECT_PARSE_OK(parseLiteral_("\"test\""));

	EXPECT_PARSE_ERROR(parseLiteral_("1 3"));
	EXPECT_PARSE_ERROR(parseLiteral_("x"));
}

/// <summary>
/// Tests for 'parseTuple' function
/// </summary>
TEST(Parser, parseTuple)
{
	auto parseTuple_ = [](const char* code)
	{
		return checkAllParsed(code, parseTuple);
	};

	EXPECT_PARSE_OK(parseTuple_("()"));
	EXPECT_PARSE_OK(parseTuple_("(1)"));
	EXPECT_PARSE_OK(parseTuple_("(1,2,a,c)"));
	EXPECT_PARSE_OK(parseTuple_("(a, (b+c, fn(r)), 8.96)"));

	EXPECT_PARSE_ERROR(parseTuple_("1,3"));
	EXPECT_PARSE_ERROR(parseTuple_("(1,3"));
	EXPECT_PARSE_ERROR(parseTuple_("a"));
	EXPECT_PARSE_ERROR(parseTuple_("3"));
}

/// <summary>
/// Tests for 'parseIdentifier' function
/// </summary>
TEST(Parser, parseIdentifier)
{
	auto parseIdentifier_ = [](const char* code)
	{
		return checkAllParsed(code, parseIdentifier);
	};

	EXPECT_PARSE_OK(parseIdentifier_("a"));
	EXPECT_PARSE_OK(parseIdentifier_("Xe45"));
	EXPECT_PARSE_OK(parseIdentifier_("CamelCase"));
	EXPECT_PARSE_OK(parseIdentifier_("a2"));
	EXPECT_PARSE_OK(parseIdentifier_("a'"));
	EXPECT_PARSE_OK(parseIdentifier_("a''"));
	EXPECT_PARSE_OK(parseIdentifier_("_a_"));

	EXPECT_PARSE_ERROR(parseIdentifier_("1a"));
	EXPECT_PARSE_ERROR(parseIdentifier_("for"));
	EXPECT_PARSE_ERROR(parseIdentifier_("if"));
	EXPECT_PARSE_ERROR(parseIdentifier_("else"));

	auto r = parseIdentifier_("test1'");

	EXPECT_EQ(AST_IDENTIFIER, r.result->getType());
	EXPECT_STREQ("test1'", r.result->getName().c_str());
}

/// <summary>
/// Tests for 'parseFunctionDef' function
/// </summary>
TEST(Parser, parseFunctionDef)
{
	auto parseFunctionDef_ = [](const char* code)
	{
		return checkAllParsed(code, parseFunctionDef);
	};

	EXPECT_PARSE_OK(parseFunctionDef_("function (x:int):int {x*2}"));
	EXPECT_PARSE_OK(parseFunctionDef_("function (x:int):int x*2"));
	EXPECT_PARSE_OK(parseFunctionDef_("function testFn(x:int):int {x*2}"));
	EXPECT_PARSE_OK(parseFunctionDef_(
		"function testFn(x:int)\n"
		"{\n"
		"  print(x);\n"
		"}"
	));
	EXPECT_PARSE_OK(parseFunctionDef_(
		"function testFn (a:float, b:float): float {(a + b) /2}"
	));

	EXPECT_PARSE_ERROR(parseFunctionDef_("(x:int):int {x*2}"));
	EXPECT_PARSE_ERROR(parseFunctionDef_("function (x:int):int"));

	auto parseR = parseFunctionDef_("function testFn (a:float, b:float): float {(a + b) /2}");
	auto r = parseR.result;

	EXPECT_EQ(AST_FUNCTION, r->getType());
	EXPECT_STREQ("testFn", r->getName().c_str());
	
	auto& children = r->children();
	ASSERT_EQ(3, children.size());
	EXPECT_EQ(AST_TUPLE_DEF, children[0]->getType());
	EXPECT_EQ(AST_IDENTIFIER, children[1]->getType());
	EXPECT_EQ(AST_BLOCK, children[2]->getType());

	EXPECT_EQ(2, children[0]->children().size());
}


/// <summary>
/// Tests for 'parsePrimaryExpr' function
/// </summary>
TEST(Parser, parsePrimaryExpr)
{
	auto parsePrimaryExpr_ = [](const char* code)
	{
		return checkAllParsed(code, parsePrimaryExpr);
	};

	EXPECT_PARSE_OK(parsePrimaryExpr_("0x33"));
	EXPECT_PARSE_OK(parsePrimaryExpr_("test"));
	EXPECT_PARSE_OK(parsePrimaryExpr_("(a+b)"));
	EXPECT_PARSE_OK(parsePrimaryExpr_("{c=a+b; c*2;}"));
	EXPECT_PARSE_OK(parsePrimaryExpr_("(1,2,3)"));

	EXPECT_PARSE_ERROR(parsePrimaryExpr_("9+4"));
}

/// <summary>
/// Tests for 'parseTypedef' function
/// </summary>
TEST(Parser, parseTypedef)
{
	auto parseTypedef_ = [](const char* code)
	{
		return checkAllParsed(code, parseTypedef);
	};

	EXPECT_PARSE_OK(parseTypedef_("type integer is int"));
	EXPECT_PARSE_OK(parseTypedef_("type vec2f is (float, float)"));
	EXPECT_PARSE_OK(parseTypedef_("type test is (a:float, b:float=7.0, c=\"test\")"));

	EXPECT_PARSE_ERROR(parseTypedef_("vec2f is (float, float)"));
	EXPECT_PARSE_ERROR(parseTypedef_("type vec2f (float, float)"));
	EXPECT_PARSE_ERROR(parseTypedef_("type integer is 17"));
}
