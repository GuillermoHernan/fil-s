/// <summary>
/// Tests for 'C' code generator
/// </summary>

#include "libfilsc_test_pch.h"
#include "c_codeGenerator_internal.h"
#include "codeGeneratorState.h"
#include "semanticAnalysis.h"

using namespace std;

/// <summary>
/// Fixture for code generation tests.
/// </summary>
class C_CodegenTests : public ::testing::Test {
protected:
	CodeGeneratorState*	m_pState = NULL;

	/// <summary>
	/// Helper function to run code generation of a chunk of code.
	/// </summary>
	/// <param name="code"></param>
	/// <returns></returns>
	string runCodegen(const char* code)
	{
		auto parseRes = parseScript(code);

		if (!parseRes.ok())
			throw parseRes.errorDesc;

		auto semanticRes = semanticAnalysis(parseRes.result);

		if (!semanticRes.ok())
			throw semanticRes.errors[0];	//Just the first error, as it is not the tested subsystem.

		return generateCode(semanticRes.ast);
	}

	~C_CodegenTests()
	{
		delete m_pState;
	}
};

/// <summary>
/// Tests 'generateCode' function.
/// </summary>
TEST_F(C_CodegenTests, generateCode)
{
	string res = runCodegen("function add (a:int, b: int):int {a+b}");

	//TODO: this is a very basic check. Add more checks.
	ASSERT_TRUE(!res.empty());
}
