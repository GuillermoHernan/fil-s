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
	bool		m_printAST = false;

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

		if (m_printAST)
			printAST(semanticRes.ast, cout);

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
	//cout << res;
}

/// <summary>
/// Test code generation for function definition nodes.
/// </summary>
TEST_F(C_CodegenTests, functionCodegen)
{
	//m_printAST = true;
	string res = runCodegen(
		"function add (a:int, b: int):int {\n"
		"  a+b\n"
		"}"
	);

	ASSERT_TRUE(!res.empty());
	//cout << res;

	EXPECT_TRUE(res.find("Parameters for 'add'") != string::npos);
	EXPECT_FALSE(res.find("Return value for 'add'") != string::npos);
	EXPECT_TRUE(res.find("Code for 'add'") != string::npos);
	EXPECT_TRUE(res.find("static int add_") != string::npos);

	res = runCodegen(
		"function add (a:int, b: int) {\n"
		"  a+b\n"
		"}"
	);

	ASSERT_TRUE(!res.empty());
	//cout << res;

	EXPECT_TRUE(res.find("Parameters for 'add'") != string::npos);
	EXPECT_FALSE(res.find("Return value for 'add'") != string::npos);
	EXPECT_TRUE(res.find("Code for 'add'") != string::npos);
	EXPECT_TRUE(res.find("static int add_") != string::npos);

	res = runCodegen(
		"function add (a:int, b: int):(int, int) {\n"
		"  (a+b, a-b)\n"
		"}"
	);

	ASSERT_TRUE(!res.empty());
	//cout << res;

	EXPECT_TRUE(res.find("Parameters for 'add'") != string::npos);
	EXPECT_TRUE(res.find("Return value for 'add'") != string::npos);
	EXPECT_TRUE(res.find("Code for 'add'") != string::npos);

	res = runCodegen(
		"function test ():() {\n"
		"  \n"
		"}"
	);

	ASSERT_TRUE(!res.empty());
	//cout << res;

	EXPECT_FALSE(res.find("Parameters for 'test'") != string::npos);
	EXPECT_FALSE(res.find("Return value for 'test'") != string::npos);
	EXPECT_TRUE(res.find("Code for 'test'") != string::npos);
}


/// <summary>
/// Test code generation for block nodes.
/// </summary>
TEST_F(C_CodegenTests, blockCodegen)
{
	m_printAST = true;

	string res = runCodegen(
		"function ppc() {\n"
		"  const alpha = (9 * 9) - 8\n"
		"  const bravo = 5 + 12\n"
		"  alpha % bravo"
		"}"
	);

	ASSERT_TRUE(!res.empty());
	cout << res;

	EXPECT_TRUE(res.find("static int ppc_") != string::npos);
	EXPECT_TRUE(res.find("alpha % bravo") != string::npos);
}
