/// <summary>
/// Tests for 'C' code generator
/// </summary>

#include "libfilsc_test_pch.h"
#include "c_codeGenerator_internal.h"
#include "codeGeneratorState.h"
#include "semanticAnalysis.h"
#include "utils.h"

//#include <time.h>

using namespace std;

/// <summary>
/// Fixture for code generation tests.
/// </summary>
class C_CodegenTests : public ::testing::Test {
protected:
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
		bool actorMode = false;

		auto entryPointFn = [&actorMode](Ref<AstNode> node) {
			if (node->getType() == AST_FUNCTION && node->getName() == "test")
				return true;
			else if (node->getType() == AST_ACTOR && node->getName() == "Test")
			{
				actorMode = true;
				return true;
			}
			else
				return false;
		};
		//clock_t		t0 = clock();
		auto parseRes = parseScript(code);

		if (!parseRes.ok())
			throw parseRes.errorDesc;
		writeAST(parseRes.result, name);

		auto semanticRes = semanticAnalysis(parseRes.result);

		if (!semanticRes.ok())
			throw semanticRes.errors[0];	//Just the first error, as it is not the tested subsystem.

		writeAST(semanticRes.result, name);

		string Ccode = generateCode(semanticRes.result, entryPointFn);

		writeCcode(Ccode, name, actorMode);
		_flushall();	//To ensure all generated files are written to the disk.

		//clock_t t1 = clock();
		//cout << "FIL-S compile time: " << double(t1 - t0)*1000 / CLOCKS_PER_SEC << " msegs.\n";
		//t0 = t1;

		compileC(name);
		//t1 = clock();
		//cout << "'C' compile time: " << double(t1 - t0)*1000 / CLOCKS_PER_SEC << " msegs.\n";
		//t0 = t1;

		int result = executeProgram(name);
		//t1 = clock();
		//cout << "execution time: " << double(t1 - t0)*1000 / CLOCKS_PER_SEC << " msegs.\n";

		return result;
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
		string path = buildOutputFilePath(testName, ".result");
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
	void writeCcode(const std::string& code, const char* testName, bool actorMode)
	{
		static const char * prolog =
			"#include <stdio.h>\n"
			"typedef unsigned char bool;\n"
			"static const bool true = 1;\n"
			"static const bool false = 0;\n"
			"//************ Prolog\n"
			"\n";
		static const char * epilog =
			"\n//************ Epilog\n"
			"int main()\n"
			"{\n"
			"	int result = test();\n"
			"	printf (\"%d\\n\", result);\n"
			"	\n"
			"	return result;\n"
			"}\n";
		static const char * epilogActor =
			"\n//************ Epilog\n"
			"\n"
			"Test mainActor;\n"
			"\n"
			"int main()\n"
			"{\n"
			//"	printf (\"%d\\n\", result);\n"
			//"	\n"
			//"	return result;\n"
			"  return 0;\n"
			"}\n";

		//TODO: How do we run actors???

		string path = buildOutputFilePath(testName, ".c");

		string fullCode = prolog + code;

		if (actorMode)
			fullCode += epilogActor;
		else
			fullCode += epilog;

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
		{
			string message = string("Error compiling 'C' code (") + testName + ".c)";
			throw exception(message.c_str());
		}
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
			"cl %s.c -nologo /FAs /Ox >%s.compiler.out\n";

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
	int res = runTest("generate1", "function test ():int {0}");

	//TODO: this is a very basic check. Add more checks.
	ASSERT_EQ(0, res);
}

/// <summary>
/// Test code generation for function definition nodes.
/// Also tests 'genFunctionHeader' function.
/// </summary>
TEST_F(C_CodegenTests, functionCodegen)
{
	//Function with a scalar return value.
	EXPECT_RUN_OK("function1",
		"function add (a:int, b: int):int {\n"
		"  a+b\n"
		"}\n"
		"function test ():int {\n"
		"  if (add(3,7) == 10) 0 else 1\n"
		"}"
	);

	//Function with no return value
	EXPECT_RUN_OK("function2",
		"function t2 (a:int, b: int):() {\n"
		"  a+b\n"
		"}\n"
		"function test ():int {\n"
		"  t2(3,1);\n"
		"  0\n"
		"}"
	);

	//Function with a tuple return value.
	EXPECT_RUN_OK("function3",
		"function stats (a:int, b: int, c: int):(sum: int, mean:int) {\n"
		"  const sum = a+b+c\n"
		"  (sum, sum/3)"
		"}\n"
		"function test ():int {\n"
		"  const result = stats (7,5,6);"
		"  if ((result.sum == 18) && (result.mean == 6))\n"
		"    0 else 1\n"
		"}"
	);
}


/// <summary>
/// Test code generation for block nodes.
/// </summary>
TEST_F(C_CodegenTests, blockCodegen)
{
	EXPECT_RUN_OK("block1",
		"function ppc(p:int, p':int) {\n"
		"  const alpha = (p * p) - 8\n"
		"  const bravo = p' + {12}\n"
		"  {}\n"
		"  alpha % bravo\n"
		"}\n"
		"function test ():int {\n"
		"  if (ppc(9,5) == 5) 0 else 1\n"
		"}\n"
	);
}


/// <summary>
/// Test code generation for Tuples.
/// </summary>
TEST_F(C_CodegenTests, tupleCodegen)
{
	EXPECT_RUN_OK("tuple1",
		"function divide(a:int, b:int) {\n"
		"  (a/b, a%b)\n"
		"}\n"
		"function doNothing() {\n"
		"  ()\n"
		"}\n"
		"function test ():int {\n"
		"  var x:(q:int, r:int)\n"
		"  doNothing()\n"
		"  x = divide (23, 7)\n"
		"  if ( (x.q == 3) && (x.r == 2)) 0 else 1\n"
		"}\n"
	);
}


/// <summary>
/// Test code generation for return statements.
/// </summary>
TEST_F(C_CodegenTests, returnCodegen)
{
	EXPECT_RUN_OK("return1",
		"function divide(a:int, b:int) {\n"
		"  return (a/b, a%b)\n"
		"}\n"
		"function divide'(a:int, b:int):(r:int, q:int) {\n"
		"  return (a/b, a%b)\n"
		"}\n"
		"function doNothing() {\n"
		"  return;\n"
		"}\n"
		"function test ():int {\n"
		"  var x:(q:int, r:int)\n"
		"  doNothing()\n"
		"  x = divide (23, 7)\n"
		"  if ( (x.q != 3) || (x.r != 2)) return 1\n"
		"  if ( divide'(14,2).r != 7) return 2\n"
		"  if ( divide'(31,7).q != 3) return 3\n"
		"\n"
		"  return 0"
		"}\n"
	);
}

/// <summary>
/// Test code generation for return assignment expressions.
/// </summary>
TEST_F(C_CodegenTests, assignmentCodegen)
{
	EXPECT_RUN_OK("assign1",
		"function test ():int {\n"
		"  var x:int\n"
		"  var y:int\n"
		"  x = y = 5\n"
		"  if ((x*y) != 25) return 1\n"
		"  \n"
		"  var t:(var a:int, var b:int)\n"
		"  t.a = 7;\n"
		"  if (t.a != 7) return 2\n"
		"  \n"
		"  t.a = x = t.b = 8;\n"
		"  const p = x * t.a * t.b;\n"
		"  if (p != 512) return 3\n"
		"  \n"
		"  return 0\n"
		"}\n"
	);
}

/// <summary>
/// Test code generation for function call nodes.
/// </summary>
TEST_F(C_CodegenTests, callCodegen)
{
	EXPECT_RUN_OK("call1",
		"function double(a:int):int {\n"
		"  a * 2\n"
		"}\n"
		"function doNothing() {\n"
		"  return;\n"
		"}\n"
		"function divide(a:int, b:int):(q:int, r:int) {\n"
		"  (a/b, a%b)\n"
		"}\n"
		"function test ():int {\n"
		"  doNothing()\n"
		"  if (double(4) != 8) return 1\n"
		"  if (divide(17, 5).q != 3) return 2\n"
		"  if (divide(30, 5).r != 0) return 3\n"
		"  \n"
		"  return 0\n"
		"}\n"
	);
}


/// <summary>
/// Test code generation for literal expressions.
/// Also tests 'varAccessCodegen'
/// </summary>
TEST_F(C_CodegenTests, literalCodegen)
{
	EXPECT_RUN_OK("literal1",
		"function test ():int {\n"
		"  var x:int\n"
		"  x = 3\n"
		"  7\n"
		"  x\n"
		"  if (x!=3) return 1\n"
		"  0\n"
		"}\n"
	);
}


/// <summary>
/// Test code generation for prefix operators.
/// </summary>
TEST_F(C_CodegenTests, prefixOpCodegen)
{
	EXPECT_RUN_OK("prefix1",
		"function test ():int {\n"
		"  var x:int\n"
		"  var y:int\n"
		"  x = 3\n"
		"  y = -x\n"
		"  if ((x+y) != 0) return 1001\n"
		"  if (!((x+y) == 0)) return 1002\n"
		"  y = ++x\n"
		"  if (y != 4) return 1003\n"
		"  if (x != y) return 1031\n"
		"  x = --y\n"
		"  if ((x != 3) || (x != y)) return 1004\n"
		"  \n"
		"  ++y;\n"
		"  if (y != 4) return 1005\n"
		"  0\n"
		"}\n"
	);
}

/// <summary>
/// Test code generation for postfix operators.
/// </summary>
TEST_F(C_CodegenTests, postfixOpCodegen)
{
	EXPECT_RUN_OK("postfix1",
		"function test ():int {\n"
		"  var x:int = 5\n"
		"  var y:int\n"
		"  y = x++\n"
		"  if (y != 5) return 1010\n"
		"  if (x != 6) return 1020\n"
		"  \n"
		"  x = y--\n"
		"  if (y != 4) return 1030\n"
		"  if (x != 5) return 1040\n"
		"  \n"
		"  ++y;\n"
		"  if (y != 5) return 1050\n"
		"  0\n"
		"}\n"
	);
}

/// <summary>
/// Test actor code generation
/// </summary>
TEST_F(C_CodegenTests, actors)
{
	EXPECT_RUN_OK("actors1",
		"actor Test {\n"
		"  output o1(int)\n"
		"  input i1(a: int) {o1(a * 2)}\n"
		"}\n"
	);
}
