/// <summary>
/// Tests FIL-S semantic analyzer.
/// </summary>

#include "libfilsc_test_pch.h"
#include "semanticAnalysis_internal.h"


/// <summary>
/// Tests 'semanticAnalysis' function.
/// </summary>
TEST(SemanticAnalysis, semanticAnalysis)
{
	const char* goodCode = "function add (a:int, b:int) {a+b}\n";

	const char* badCode = "function add (a:wrongType, b:wrongType) {a+b}\n";

	EXPECT_SEM_OK(semAnalysisCheck(goodCode));
	EXPECT_SEM_ERROR(semAnalysisCheck(badCode));
}
