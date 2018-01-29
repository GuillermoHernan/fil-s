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
/// Tests 'isBinaryOp' function, which checks if an operator is an binary operator.
/// </summary>
TEST(Parser, isBinaryOp)
{
	const char * binaryOps = ">>> >> << ** + - * / % & | && || ^ < > >= <= == != ";
	const char * otherOps = "-= += ~ !";

	LexToken	tok(binaryOps);

	for (tok = tok.next(); !tok.eof(); tok = tok.next())
		EXPECT_TRUE(isBinaryOp(tok));

	tok = LexToken(otherOps);

	for (tok = tok.next(); !tok.eof(); tok = tok.next())
		EXPECT_FALSE(isBinaryOp(tok));
}

