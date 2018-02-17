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

/// <summary>Tests 'declarationTypeCheck' function.
/// Also tests 'recursiveSymbolReferenceCheck' function.</summary>
TEST(TypeCheck, declarationTypeCheck)
{
	auto findDeclaration = [](Ref<AstNode> root){
		return findNode(root, AST_DECLARATION); 
	};

	auto r = semAnalysisCheck("const a:int;");

	ASSERT_SEM_OK(r);
	auto decl = findDeclaration(r.ast);
	EXPECT_DATATYPE_STR("int", decl);

	EXPECT_SEM_OK(semAnalysisCheck("const a:int = 5;"));
	EXPECT_SEM_ERROR(semAnalysisCheck("const a:int = true;"));

	//Inferred type
	r = semAnalysisCheck("const a = (true, 4);");

	ASSERT_SEM_OK(r);
	decl = findDeclaration(r.ast);
	EXPECT_DATATYPE_STR("(bool,int)", decl);

	r = semAnalysisCheck("const a;");
	ASSERT_SEM_ERROR(r);
	EXPECT_EQ (ETYPE_DECLARATION_WITHOUT_TYPE, r.errors[0].type());

	r = semAnalysisCheck("const a = a + 1");

	ASSERT_SEM_ERROR(r);
	EXPECT_EQ(ETYPE_RECURSIVE_SYMBOL_REFERENCE_1, r.errors[0].type());
}


/// <summary>Tests 'ifTypeCheck' function.</summary>
TEST(TypeCheck, ifTypeCheck)
{
	auto r = semAnalysisCheck("const a = if (1>7) (4,5) else (6,7);");

	ASSERT_SEM_OK(r);
	auto node = findNode(r.ast, AST_IF);
	EXPECT_DATATYPE_STR("(int,int)", node);

	r = semAnalysisCheck("const a = if (false) (4,5) else (false,7);");

	ASSERT_SEM_OK(r);
	node = findNode(r.ast, AST_IF);
	EXPECT_DATATYPE_STR("()", node);

	r = semAnalysisCheck("const a = if (true) 4;");

	ASSERT_SEM_OK(r);
	node = findNode(r.ast, AST_IF);
	EXPECT_DATATYPE_STR("()", node);

	r = semAnalysisCheck("const a = if (0) 3 else 5;");

	ASSERT_SEM_ERROR(r);
	EXPECT_EQ(ETYPE_WRONG_IF_CONDITION_TYPE_1, r.errors[0].type());
}

/// <summary>Tests 'returnTypeAssign' function.</summary>
TEST(TypeCheck, returnTypeAssign)
{
	auto r = semAnalysisCheck("function f() {return (9,4);}");

	ASSERT_SEM_OK(r);
	auto node = findNode(r.ast, AST_RETURN);
	EXPECT_DATATYPE_STR("(int,int)", node);

	r = semAnalysisCheck("function f() {return;}");

	ASSERT_SEM_OK(r);
	node = findNode(r.ast, AST_RETURN);
	EXPECT_DATATYPE_STR("()", node);
}

/// <summary>Tests 'functionDefTypeCheck' function.</summary>
TEST(TypeCheck, functionDefTypeCheck)
{
	//Function with declared type and matching body type.
	auto r = semAnalysisCheck("function div(a: int, b: int):(int, int){\n"
		"const x = a/b\n"
		"const r = a - (x*b)\n"
		"(x,r)}");

	//printAST(r.ast, cout, 0);
	ASSERT_SEM_OK(r);
	auto node = findNode(r.ast, AST_FUNCTION);
	EXPECT_DATATYPE_STR("function(int,int):(int,int)", node);

	//Function with declared type and mismatching body type.
	r = semAnalysisCheck("function div(a: int, b: int):int {\n"
		"const x = a/b\n"
		"const r = a - (x*b)\n"
		"(x,r)}");

	ASSERT_SEM_ERROR(r);
	EXPECT_EQ(ETYPE_INCOMPATIBLE_TYPES_2, r.errors[0].type());

	//Return value inference.
	r = semAnalysisCheck("function div(a: int, b: int){\n"
		"const x = a/b\n"
		"const r = a - (x*b)\n"
		"(x,r,r<0)}");

	ASSERT_SEM_OK(r);
	node = findNode(r.ast, AST_FUNCTION);
	EXPECT_DATATYPE_STR("function(int,int):(int,int,bool)", node);
}

/// <summary>Tests 'assignmentTypeCheck' function.</summary>
TEST(TypeCheck, assignmentTypeCheck)
{
	auto r = semAnalysisCheck("function f() {var a:int; a = 7;}");
	ASSERT_SEM_OK(r);

	auto node = findNode(r.ast, AST_ASSIGNMENT);
	ASSERT_TRUE(node.notNull());
	EXPECT_DATATYPE_STR("int", node);

	ASSERT_SEM_ERROR ( semAnalysisCheck("function f() {var a:bool; a = 7;}"));
}

/// <summary>Tests 'callTypeCheck' function.</summary>
TEST(TypeCheck, callTypeCheck)
{
	auto r = semAnalysisCheck("function add(a:int, b:int):int {a+b}\n"
		"function test():() {const x = add(5,7);}");
	ASSERT_SEM_OK(r);

	//printAST(r.ast, cout);

	auto node = findNode(r.ast, AST_FNCALL);
	ASSERT_TRUE(node.notNull());
	EXPECT_DATATYPE_STR("int", node);
}

/// <summary>Tests 'memberAccessTypeCheck' function.</summary>
TEST(TypeCheck, memberAccessTypeCheck)
{
	auto r = semAnalysisCheck(
		"type TestTuple is (a:int, b:int)\n"
		"function test():int {\n"
		"  const x:TestTuple = (9,5)\n"
		"  return x.a + x.b\n"
		"}"
	);
	//printAST(r.ast, cout);
	ASSERT_SEM_OK(r);

	auto node = findNode(r.ast, AST_MEMBER_ACCESS);
	ASSERT_TRUE(node.notNull());
	EXPECT_DATATYPE_STR("int", node);
	EXPECT_DATATYPE_STR("(int,int)", node->child(0));

	r = semAnalysisCheck(
		"type TestTuple is (a:int, n:int)\n"
		"function test():int {\n"
		"  const x:TestTuple = (9,5)\n"
		"  return x.a + x.b\n"
		"}"
	);
	ASSERT_SEM_ERROR(r);
	EXPECT_EQ(ETYPE_MEMBER_NOT_FOUND_2, r.errors[0].type());
}
