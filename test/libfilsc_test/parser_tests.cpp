/// <summary>
/// Tests FIL-S parser.
/// </summary>

#include "libfilsc_test_pch.h"
#include "parser.h"
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