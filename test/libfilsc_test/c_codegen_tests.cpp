/// <summary>
/// Tests for 'C' code generator
/// </summary>

#include "libfilsc_test_pch.h"
#include "c_codeGenerator_internal.h"
#include "codeGeneratorState.h"
#include "semanticAnalysis.h"
#include "utils.h"

using namespace std;

/// <summary>
/// Fixture for code generation tests.
/// </summary>
class C_CodegenTests : public ::testing::Test {
protected:
	CodeGeneratorState*	m_pState = NULL;
	string		m_resultsDir;

	/// <summary>Test initialization</summary>
	virtual void SetUp()override
	{
		m_resultsDir = makeResultsDir();
	}

	/// <summary>
	/// Compiles (both FIL-S and 'C') and executes a test script.
	/// </summary>
	/// <param name="name"></param>
	/// <param name="code"></param>
	/// <returns></returns>
	int runTest(const char* name, const char* code)
	{
		auto entryPointFn = [](Ref<AstNode> node) {
			return (node->getType() == AST_FUNCTION && node->getName() == "test");
		};
		auto parseRes = parseScript(code);

		if (!parseRes.ok())
			throw parseRes.errorDesc;

		auto semanticRes = semanticAnalysis(parseRes.result);

		if (!semanticRes.ok())
			throw semanticRes.errors[0];	//Just the first error, as it is not the tested subsystem.

		writeAST(semanticRes.ast, name);

		string Ccode = generateCode(semanticRes.ast, entryPointFn);

		writeCcode(Ccode, name);
		_flushall();	//To ensure all generated files are witten to the disk.

		compileC(name);
		return executeProgram(name);
	}

	~C_CodegenTests()
	{
		delete m_pState;
	}

private:

	/// <summary>
	/// Creates the directory for test results, if it does not exist.
	/// </summary>
	/// <returns></returns>
	static std::string makeResultsDir()
	{
		auto testInfo = ::testing::UnitTest::GetInstance()->current_test_info();
		string path =string( "results/") + testInfo->test_case_name() + "." + testInfo->name();

		if (!createDirIfNotExist(path))
		{
			string message = "Cannot create directory: " + path;
			throw exception(message.c_str());
		}

		return path;
	}

	/// <summary>
	/// Writes the AST to a file in the test directory;
	/// </summary>
	/// <param name="node"></param>
	/// <param name="testName"></param>
	void writeAST(Ref<AstNode> node, const char* testName)
	{
		string path = buildOutputFilePath(testName, ".ast");
		string content = printAST(node);

		if (!writeTextFile(path, content))
		{
			string message = "Cannot write file: " + path;
			throw exception(message.c_str());
		}
	}

	/// <summary>
	/// Writes the 'C' code to a file in the test directory;
	/// </summary>
	/// <param name="code"></param>
	/// <param name="testName"></param>
	void writeCcode(const std::string& code, const char* testName)
	{
		static const char * prolog =
			"#include <stdio.h>\n"
			"//************ Prolog\n"
			"\n";
		static const char * epilog =
			"//************ Epilog\n"
			"int main()\n"
			"{\n"
			"	int result = test();\n"
			"	printf (\"%d\\n\", result);\n"
			"	\n"
			"	return result;\n"
			"}\n";

		string path = buildOutputFilePath(testName, ".c");

		string fullCode = prolog + code + epilog;

		if (!writeTextFile(path, fullCode))
		{
			string message = "Cannot write file: " + path;
			throw exception(message.c_str());
		}
	}

	/// <summary>
	/// Compiles the resulting 'C' file with an external compiler.
	/// </summary>
	/// <param name="testName"></param>
	int executeProgram(const char* testName)
	{
		//TODO: The executable extension is also system-dependent.
		string path = buildOutputFilePath(testName, ".exe");
		path = normalizePath(path);
		string command = "cmd /C \"" + path + "\" >\"" + path + ".out\"";

		return system(command.c_str());
	}

	void compileC(const char* testName)
	{
		string scriptPath = createCompileScript(testName);

		string command = "cmd /C \"" + scriptPath + "\" >NUL";

		_flushall();
		int result = system(command.c_str());
		if (result != 0)
			throw exception("Error compiling 'C' code");
	}

	std::string buildOutputFilePath(const string& testName, const string& extension)
	{
		return m_resultsDir + "/" + testName + extension;
	}

	string createCompileScript(const char* testName)
	{
		//TODO: This function is very system dependent.
		static const char* base =
			"call \"H:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\VC\\Auxiliary\\Build\\vcvars32.bat\"\n"
			"cd \"%s\"\n"
			"%s\n"
			"cl %s.c -nologo >%s.compiler.out\n";

		char buffer[4096];
		string absPath = joinPaths(getCurrentDirectory(), m_resultsDir);
		string drive = absPath.substr(0, 2);
		string scriptPath = buildOutputFilePath(testName, ".compile.bat");

		absPath = normalizePath(absPath);
		scriptPath = normalizePath(scriptPath);

		sprintf_s(buffer, base, absPath.c_str(), drive.c_str(), testName, testName);

		if (!writeTextFile(scriptPath, buffer))
		{
			string message = "Cannot write file: " + scriptPath;
			throw exception(message.c_str());
		}

		return scriptPath;
	}
}; //class C_CodegenTests

#define EXPECT_RUN_OK(n,x) EXPECT_EQ(0, runTest((n),(x)))

/// <summary>
/// Tests 'generateCode' function.
/// </summary>
TEST_F(C_CodegenTests, generateCode)
{
	int res = runTest("test1", "function test ():int {0}");

	//TODO: this is a very basic check. Add more checks.
	ASSERT_EQ(0, res);
}

/// <summary>
/// Test code generation for function definition nodes.
/// </summary>
TEST_F(C_CodegenTests, functionCodegen)
{
	EXPECT_RUN_OK ( "test1", 
		"function add (a:int, b: int):int {\n"
		"  a+b\n"
		"}\n"
		"function test ():int {\n"
		"  if (add(3,7) == 10) 0 else 1\n"
		"}"
	);

	//ASSERT_TRUE(!res.empty());
	////cout << res;

	//EXPECT_TRUE(res.find("Parameters for 'add'") != string::npos);
	//EXPECT_FALSE(res.find("Return value for 'add'") != string::npos);
	//EXPECT_TRUE(res.find("Code for 'add'") != string::npos);
	//EXPECT_TRUE(res.find("static int add_") != string::npos);
	//EXPECT_TRUE(res.find("return") != string::npos);

	//res = runCodegen(
	//	"function add (a:int, b: int) {\n"
	//	"  a+b\n"
	//	"}"
	//);

	//ASSERT_TRUE(!res.empty());
	////cout << res;

	//EXPECT_TRUE(res.find("Parameters for 'add'") != string::npos);
	//EXPECT_FALSE(res.find("Return value for 'add'") != string::npos);
	//EXPECT_TRUE(res.find("Code for 'add'") != string::npos);
	//EXPECT_TRUE(res.find("static int add_") != string::npos);

	//res = runCodegen(
	//	"function add (a:int, b: int):(int, int) {\n"
	//	"  (a+b, a-b)\n"
	//	"}"
	//);

	//ASSERT_TRUE(!res.empty());
	//cout << res;

	//EXPECT_TRUE(res.find("Parameters for 'add'") != string::npos);
	//EXPECT_TRUE(res.find("Return value for 'add'") != string::npos);
	//EXPECT_TRUE(res.find("Code for 'add'") != string::npos);

	//res = runCodegen(
	//	"function test ():() {\n"
	//	"  \n"
	//	"}"
	//);

	//ASSERT_TRUE(!res.empty());
	////cout << res;

	//EXPECT_FALSE(res.find("Parameters for 'test'") != string::npos);
	//EXPECT_FALSE(res.find("Return value for 'test'") != string::npos);
	//EXPECT_TRUE(res.find("Code for 'test'") != string::npos);
}


/// <summary>
/// Test code generation for block nodes.
/// </summary>
TEST_F(C_CodegenTests, DISABLED_blockCodegen)
{
	//m_printAST = true;

	//string res = runCodegen(
	//	"function ppc() {\n"
	//	"  const alpha = (9 * 9) - 8\n"
	//	"  const bravo = 5 + 12\n"
	//	"  alpha % bravo"
	//	"}"
	//);

	//ASSERT_TRUE(!res.empty());
	//cout << res;

	//EXPECT_TRUE(res.find("static int ppc_") != string::npos);
	//EXPECT_TRUE(res.find("alpha % bravo") != string::npos);
}
