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

/// <summary>
/// Tests the node creation function.
/// </summary>
TEST(AstNode, create)
{
    auto node = AstNode::create(
        AST_FUNCTION,
        ScriptPosition(SourceFilePtr(), 1, 2),
        "test_function",
        "test_value",
        ASTF_CONST
    );

    EXPECT_EQ(AST_FUNCTION, node->getType());
    EXPECT_EQ(1, node->position().line());
    EXPECT_EQ(2, node->position().column());
    EXPECT_STREQ("test_function", node->getName().c_str());
    EXPECT_STREQ("test_value", node->getValue().c_str());
    EXPECT_TRUE(node->hasFlag(ASTF_CONST));
    EXPECT_EQ(ASTF_CONST, node->getFlags());
}
