/// <summary>
/// Tests for the type check compiler pass.
/// </summary>

#include "libfilsc_test_pch.h"
#include "typeCheckPass.h"
#include "semAnalysisState.h"
#include "semanticAnalysis.h"
#include "dataTypes.h"

using namespace std;

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

/// <summary>Tests 'tupleDefTypeCheck' function.</summary>
TEST(TypeCheck, tupleDefTypeCheck)
{
	auto r = semAnalysisCheck(
		"type test is (\n"
		"  a:int,\n"
		"  b:bool,\n"
		"  c:(int, int)\n"
		");"
	);

	ASSERT_SEM_OK(r);

	auto node = findNode(r.ast, [](auto node) {
		return node->getType() == AST_TUPLE_DEF;
	});

	ASSERT_TRUE(node.notNull());

	auto dataType = node->getDataType();
	EXPECT_EQ(DT_TUPLE, dataType->type());
	EXPECT_STREQ("(int,bool,(int,int))", dataType->toString().c_str());
}

/// <summary>Tests 'blockTypeCheck' function.</summary>
TEST(TypeCheck, blockTypeCheck)
{
	auto r = semAnalysisCheck("const a = {}");

	ASSERT_SEM_OK(r);

	auto nodes = findNodes(r.ast, [](auto node) {
		return node->getType() == AST_BLOCK || node->getType() == AST_DECLARATION;
	});

	ASSERT_EQ(2, nodes.size());
	EXPECT_DATATYPE(DT_VOID, nodes[0]);
	EXPECT_DATATYPE(DT_VOID, nodes[1]);

	r = semAnalysisCheck("const a = {const b = (1*4)+9; (b,false)}");

	ASSERT_SEM_OK(r);

	auto node = findNode(r.ast, [](auto node) {
		return node->getType() == AST_BLOCK;
	});

	ASSERT_TRUE(node.notNull());
	EXPECT_DATATYPE(DT_TUPLE, node);
	EXPECT_STREQ("(int,bool)", node->getDataType()->toString().c_str());

	//printAST(r.ast, cout);
}

/// <summary>Tests 'tupleTypeCheck' function.</summary>
TEST(TypeCheck, tupleTypeCheck)
{
	auto r = semAnalysisCheck("const a = (1,2,true)");

	ASSERT_SEM_OK(r);

	auto node = findNode(r.ast, [](auto node) {
		return node->getType() == AST_TUPLE;
	});

	ASSERT_TRUE(node.notNull());
	EXPECT_DATATYPE(DT_TUPLE, node);

	EXPECT_STREQ("(int,int,bool)", node->getDataType()->toString().c_str());

	//printAST(r.ast, cout);
}
