/// <summary>
/// Tests for AST classes and functions
/// </summary>

#include "libfilsc_test_pch.h"
#include "ast.h"
#include "semanticAnalysis.h"

using namespace std;

/// <summary>
/// Tests for 'astTypeToString' and 'astTypeFromString' functions
/// </summary>
TEST(AST, to_string_from_string)
{
	for (int i = 0; i < AST_TYPES_COUNT; ++i)
	{
		string str = astTypeToString((AstNodeTypes)i);
		ASSERT_STRNE("BAD_AST_TYPE", str.c_str());

		auto type = astTypeFromString(str);
		ASSERT_EQ(i, type);
	}

	ASSERT_STREQ("BAD_AST_TYPE", astTypeToString(AST_TYPES_COUNT).c_str());
	ASSERT_STREQ("BAD_AST_TYPE", astTypeToString((AstNodeTypes)-1).c_str());
}
