/// <summary>
/// Miscellaneous utilities to help testing FIL-S compiler.
/// </summary>

#pragma once

#include "gtest/gtest.h"
#include "parser.h"

#define EXPECT_PARSE_OK(x) EXPECT_TRUE(checkExprOk((x)))
#define EXPECT_PARSE_ERROR(x) EXPECT_TRUE(checkExprError(x))

#define EXPECT_SEM_OK(x) EXPECT_TRUE(checkSemOk((x)))
#define EXPECT_SEM_ERROR(x) EXPECT_TRUE(checkSemError(x))

typedef ExprResult::ParseFunction ParseFunction;

class SemanticResult;

::testing::AssertionResult checkExprOk(const ExprResult& res);
::testing::AssertionResult checkExprError(const ExprResult& res);

::testing::AssertionResult checkSemOk(const SemanticResult& res);
::testing::AssertionResult checkSemError(const SemanticResult& res);

ExprResult checkAllParsed(const char* code, ParseFunction parseFn);
SemanticResult semAnalysisCheck(const char* code);