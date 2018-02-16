/// <summary>
/// Miscellaneous utilities to help testing FIL-S compiler.
/// </summary>

#pragma once

#include "gtest/gtest.h"
#include "parser.h"
#include <functional>
#include "dataTypes.h"

#define EXPECT_PARSE_OK(x) EXPECT_TRUE(checkExprOk((x)))
#define EXPECT_PARSE_ERROR(x) EXPECT_TRUE(checkExprError(x))
#define ASSERT_PARSE_OK(x) ASSERT_TRUE(checkExprOk((x)))
#define ASSERT_PARSE_ERROR(x) ASSERT_TRUE(checkExprError(x))

#define EXPECT_SEM_OK(x) EXPECT_TRUE(checkSemOk((x)))
#define EXPECT_SEM_ERROR(x) EXPECT_TRUE(checkSemError(x))
#define ASSERT_SEM_OK(x) ASSERT_TRUE(checkSemOk((x)))
#define ASSERT_SEM_ERROR(x) ASSERT_TRUE(checkSemError(x))

typedef ExprResult::ParseFunction ParseFunction;

class SemanticResult;

::testing::AssertionResult checkExprOk(const ExprResult& res);
::testing::AssertionResult checkExprError(const ExprResult& res);

::testing::AssertionResult checkSemOk(const SemanticResult& res);
::testing::AssertionResult checkSemError(const SemanticResult& res);

ExprResult checkAllParsed(const char* code, ParseFunction parseFn);
SemanticResult semAnalysisCheck(const char* code);

void findNodes(Ref<AstNode> root, std::function<bool(Ref<AstNode>)> predicate, AstNodeList& result);
AstNodeList findNodes(Ref<AstNode> root, std::function<bool(Ref<AstNode>)> predicate);
Ref<AstNode> findNode(Ref<AstNode> root, std::function<bool(Ref<AstNode>)> predicate);

#define EXPECT_DATATYPE(x,y) EXPECT_EQ(x, getDataType((y)))
#define ASSERT_DATATYPE(x,y) ASSERT_EQ(x, getDataType((y)))

EDataType getDataType(Ref<AstNode> node);
EDataType getDataType(Ref<BaseType> type);

#define EXPECT_DATATYPE_STR(x,y) EXPECT_STREQ(x, getDataTypeStr((y)).c_str())
#define ASSERT_DATATYPE_STR(x,y) ASSERT_STREQ(x, getDataTypeStr((y)).c_str())

std::string getDataTypeStr(Ref<AstNode> node);
std::string getDataTypeStr(Ref<BaseType> type);

std::string printAST(Ref<AstNode> node, int indentLevel = 0);
void printAST(Ref<AstNode> node, std::ostream& output, int indentLevel = 0);
