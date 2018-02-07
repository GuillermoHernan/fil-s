/// <summary>
/// Tests for the type check compiler pass.
/// </summary>

#include "libfilsc_test_pch.h"
#include "typeCheckPass.h"
#include "semAnalysisState.h"
#include "semanticAnalysis.h"

/// <summary>
/// Tests 'typeExistsCheck' function.
/// </summary>
TEST(TypeCheck, typeExistsCheck)
{
	EXPECT_SEM_OK(semAnalysisCheck("const a:int\n const b:bool"));
	EXPECT_SEM_OK(semAnalysisCheck(
		"type integer is int\n"
		"const a:integer\n"
		"const b:bool"
	));

	auto r = semAnalysisCheck(
		"const a:integer\n"
		"const b:bool"
	);

	ASSERT_SEM_ERROR(r);
	EXPECT_EQ(ETYPE_NON_EXISTENT_SYMBOL_1, r.errors[0].type());

	r = semAnalysisCheck(
		"const integer=7\n"
		"const a:integer\n"
		"const b:bool"
	);

	ASSERT_SEM_ERROR(r);
	EXPECT_EQ(ETYPE_NOT_A_TYPE_1, r.errors[0].type());
}
