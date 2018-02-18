/// <summary>
/// Tests for 'CodeGeneratorState' class
/// </summary>

#include "libfilsc_test_pch.h"
#include "codeGeneratorState.h"
#include "semanticAnalysis.h"

using namespace std;

/// <summary>
/// Tests 'cname' function (both versions)
/// Also tests 'allocCName' function.
/// </summary>
TEST(CodeGeneratorState, cname)
{
	ostringstream		output;
	CodeGeneratorState	state(&output);

	auto r = semAnalysisCheck(
		"type entero is int\n"
		"type TestTuple is (fieldA:int, fieldB:int)\n"
		"const testValue:entero = 4\n"
	);
	ASSERT_SEM_OK(r);

	//printAST(r.ast);

	auto intType = DefaultType::createInt().staticCast<BaseType>();	//I don't know why the compiler says the
																	//call is ambigous if I do not cast it to BaseType...
																	//'template' limitations?
	EXPECT_STREQ("int", state.cname(intType).c_str());
	
	auto node = findNode(r.ast, "entero");
	ASSERT_TRUE(node.notNull());
	EXPECT_STREQ("int", state.cname(node).c_str());
	EXPECT_STREQ("int", state.cname(node->getDataType()).c_str());

	node = findNode(r.ast, "testValue");
	ASSERT_TRUE(node.notNull());
	string cname = state.cname(node);
	EXPECT_TRUE(cname.find("testValue") != string::npos);
	//cout << cname << "\n";

	string cname2 = state.cname(node);
	EXPECT_STREQ(cname.c_str(), cname2.c_str());

	node = findNode(r.ast, "TestTuple");
	ASSERT_TRUE(node.notNull());
	cname = state.cname(node);
	EXPECT_TRUE(cname.find("TestTuple") != string::npos);
	//cout << cname << "\n";
}

/// <summary>
/// Tests the temporaries / blocks functionallity of 'CodeGeneratorState' class
/// </summary>
TEST(CodeGeneratorState, temporaries)
{
	ostringstream		output;
	CodeGeneratorState	state(&output);
	string				name1, name2, name3;

	state.enterBlock();
	
	EXPECT_TRUE(state.allocTemp("int", name1));
	EXPECT_TRUE(state.allocTemp("int", name2));
	EXPECT_TRUE(state.releaseTemp(name1));
	EXPECT_FALSE(state.allocTemp("int", name3));
	EXPECT_STREQ(name1.c_str(), name3.c_str());

	state.enterBlock();
	EXPECT_FALSE(state.releaseTemp(name3));
	state.exitBlock();

	EXPECT_TRUE(state.releaseTemp(name2));
	EXPECT_TRUE(state.releaseTemp(name3));

	state.exitBlock();
}
