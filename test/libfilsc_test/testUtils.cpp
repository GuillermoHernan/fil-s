/// <summary>
/// Miscellaneous utilities to help testing FIL-S compiler.
/// </summary>

#include "libfilsc_test_pch.h"
#include "testUtils.h"
#include "semanticAnalysis.h"

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
/// Checks that a semantic analysis has succeded. If not, it  prints the error list
/// using 'google test' facilities.
/// </summary>
/// <param name="res"></param>
/// <returns></returns>
::testing::AssertionResult checkSemOk(const SemanticResult& res)
{
	if (res.ok())
		return ::testing::AssertionSuccess();
	else
	{
		auto result = ::testing::AssertionFailure() << "Found " << res.errors.size() << " errors:\n";

		for (auto error : res.errors)
			result << error.what() << "\n";

		return result;
	}
}

/// <summary>
/// Checks that the semantic analysis has errors.
/// </summary>
/// <param name="res"></param>
/// <returns></returns>
::testing::AssertionResult checkSemError(const SemanticResult& res)
{
	if (!res.ok())
		return ::testing::AssertionSuccess();
	else
		return ::testing::AssertionFailure() << "Semantic error expected";
}

/// <summary>
/// Checks that a parsing function consumes all input.
/// </summary>
/// <param name="code"></param>
/// <param name="parseFn"></param>
/// <returns></returns>
ExprResult checkAllParsed(const char* code, ParseFunction parseFn)
{
	auto r = parseFn(LexToken(code).next());

	if (r.ok() && !r.token.eof())
		r = r.getError(ETYPE_UNEXPECTED_TOKEN_2, r.token.text().c_str(), "<EOF>");
	return r;
};

/// <summary>
/// Performs a semantic analysis for a piece of code.
/// </summary>
/// <param name="code"></param>
/// <returns></returns>
SemanticResult semAnalysisCheck(const char* code)
{
	auto parseRes = parseScript(code);

	if (parseRes.error())
		return SemanticResult(parseRes.errorDesc);
	else
		return semanticAnalysis(parseRes.result);
}