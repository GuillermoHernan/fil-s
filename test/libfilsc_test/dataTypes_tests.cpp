/// <summary>
/// Tests for data types classes.
/// </summary>

#include "libfilsc_test_pch.h"
#include "dataTypes.h"
#include "semanticAnalysis.h"

/// <summary>
/// Tests 'DefaultType' creation functions.
/// </summary>
TEST(DefaultType, creation)
{
	auto dt = DefaultType::createBool();
	EXPECT_EQ(DT_BOOL, dt->type());
	EXPECT_STREQ("bool", dt->getName().c_str());

	dt = DefaultType::createInt();
	EXPECT_EQ(DT_INT, dt->type());
	EXPECT_STREQ("int", dt->getName().c_str());

	dt = DefaultType::createVoid();
	EXPECT_EQ(DT_VOID, dt->type());
	EXPECT_TRUE(dt->getName().empty());
}

/// <summary>
/// Tests 'TupleType' creation function.
/// </summary>
TEST(TupleType, creation)
{
	auto tuple = TupleType::create();

	EXPECT_EQ(DT_VOID, tuple->type());
	EXPECT_TRUE(tuple->getName().empty());

	tuple->addMember(astCreateDeclaration(ScriptPosition(), "m1", Ref<AstNode>(), Ref<AstNode>()));
	EXPECT_EQ(DT_TUPLE, tuple->type());
	EXPECT_EQ(1, tuple->memberCount());

	tuple = TupleType::create("namedTuple");
	EXPECT_STREQ("namedTuple", tuple->getName().c_str());
}

/// <summary>
/// Tests 'TupleType::findMemberByName'.
/// Also tests 'getMemberType'
/// </summary>
TEST(TupleType, findMemberByName)
{
	auto r = semAnalysisCheck("type test is (a:int, b:bool, int)");

	ASSERT_SEM_OK(r);

	auto nodes = findNodes(r.ast, [](Ref<AstNode> node)->bool {
		return node->getType() == AST_TUPLE_DEF;
	});

	ASSERT_EQ(1, nodes.size());

	auto tupleType = nodes[0]->getDataType().staticCast<TupleType>();

	EXPECT_EQ(3, tupleType->memberCount());
	EXPECT_EQ(0, tupleType->findMemberByName("a"));
	EXPECT_EQ(1, tupleType->findMemberByName("b"));
	EXPECT_EQ(-1, tupleType->findMemberByName("c"));

	EXPECT_EQ(DT_INT, tupleType->getMemberType(0)->type());
	EXPECT_EQ(DT_BOOL, tupleType->getMemberType(1)->type());
	EXPECT_EQ(DT_INT, tupleType->getMemberType(2)->type());
}

/// <summary>
/// Tests 'TupleType::toString'
/// </summary>
TEST(TupleType, toString)
{
	auto r = semAnalysisCheck("type test is (a:int, b:bool, (int,int))");

	ASSERT_SEM_OK(r);

	auto tupleNode = r.ast->child(0)->child(0);

	ASSERT_EQ(AST_TUPLE_DEF, tupleNode->getType());

	auto tupleType = tupleNode->getDataType();

	ASSERT_STREQ("(int,bool,(int,int))", tupleType->toString().c_str());
}

/// <summary>
/// Tests 'FunctionType::create' and 'toString' funcitons
/// </summary>
TEST(FunctionType, create)
{
	auto r = semAnalysisCheck("function test (a: int, b: bool):int { if (b) a else a*2 }");
	ASSERT_SEM_OK(r);
	auto fnNode = r.ast->child(0);
	ASSERT_EQ(AST_FUNCTION, fnNode->getType());
	auto fnType = fnNode->getDataType();
	ASSERT_STREQ("function(int,bool):int", fnType->toString().c_str());

	r = semAnalysisCheck("function test (a: int, b: bool) { if (b) a else b }");
	ASSERT_SEM_OK(r);
	fnNode = r.ast->child(0);
	ASSERT_EQ(AST_FUNCTION, fnNode->getType());
	fnType = fnNode->getDataType();
	ASSERT_STREQ("function(int,bool):()", fnType->toString().c_str());

	//TODO: This test should work, but it is not a bug on 'FunctionType' class.
	/*r = semAnalysisCheck("function test (a: int, b: int):(int, int) { (a*a, b*b) }");
	ASSERT_SEM_OK(r);
	fnNode = r.ast->child(0);
	ASSERT_EQ(AST_FUNCTION, fnNode->getType());
	fnType = fnNode->getDataType();
	ASSERT_STREQ("function(int,int):(int,int)", fnType->toString().c_str());*/
}
