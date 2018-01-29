/// <summary>
/// Tests FIL-S parser.
/// </summary>

#include "libfilsc_test_pch.h"
#include "parser_internal.h"
#include "compileError.h"

#define EXPECT_PARSE_OK(x) EXPECT_TRUE(checkExprOk((x)))
#define EXPECT_PARSE_ERROR(x) EXPECT_TRUE(checkExprError(x))

/// <summary>
/// Checks an expression result object, and passes the error message to 'Google test'
/// if failed.
/// </summary>
/// <param name="res"></param>
/// <returns></returns>
::testing::AssertionResult checkExprOk(const ExprResult& res)
{
	if (res.ok())
		return ::testing::AssertionSuccess();
	else
		return ::testing::AssertionFailure() << res.errorDesc.what();
}

::testing::AssertionResult checkExprError(const ExprResult& res)
{
	if (res.error())
		return ::testing::AssertionSuccess();
	else
		return ::testing::AssertionFailure() << "Compilation error expected";
}

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
/// </summary>
TEST(Parser, parseDeclaration)
{
	auto parseDeclaration_ = [](const char* code)
	{
		auto r = parseDeclaration(LexToken(code).next());
		if (r.ok() && !r.token.eof())
			r = r.getError(ETYPE_UNEXPECTED_TOKEN_2, r.token.text().c_str(), "<EOF>");
		return r;
	};

	EXPECT_PARSE_OK(parseDeclaration_("standAlone"));
	EXPECT_PARSE_OK(parseDeclaration_("typed:int"));
	EXPECT_PARSE_OK(parseDeclaration_("initialized = a"));
	EXPECT_PARSE_OK(parseDeclaration_("full:int = 3"));

	EXPECT_PARSE_ERROR(parseDeclaration_("var full:int = a"));
	EXPECT_PARSE_ERROR(parseDeclaration_("if = 3"));
	EXPECT_PARSE_ERROR(parseDeclaration_("const: int"));

}
