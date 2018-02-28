/// <summary>
/// Tests for the symbol gathering compiler pass.
/// </summary>

#include "libfilsc_test_pch.h"
#include "gatherPass.h"
#include "semAnalysisState.h"
#include "scopeCreationPass.h"

/// <summary>
/// Fixture for 'gatherpass' tests.
/// </summary>
class GatherPassTests : public ::testing::Test {
protected:
	SemAnalysisState*	m_pState = NULL;

	/// <summary>
	/// Helper function to run gather pass (performs all required initialization)
	/// </summary>
	/// <param name="code"></param>
	/// <returns></returns>
	SemanticResult runGatherPass(const char* code)
	{
		auto parseRes = parseScript(code);

		if (!parseRes.ok())
			return SemanticResult(parseRes.errorDesc);

		//reset state
		delete m_pState;
		m_pState = new SemAnalysisState();

		auto node = parseRes.result;
		auto result = scopeCreationPass(node, *m_pState);

		if (!result.ok())
			return result;

		node = result.result;
		return symbolGatherPass(node, *m_pState);
	}

	~GatherPassTests()
	{
		delete m_pState;
	}
};

/// <summary>
/// Tests 'gatherSymbol' function.
/// </summary>
TEST_F(GatherPassTests, gatherSymbol)
{
	auto r = runGatherPass("const a=7\n const b=10");

	ASSERT_TRUE(r.ok());
	EXPECT_TRUE(m_pState->rootScope->contains("a"));
	EXPECT_TRUE(m_pState->rootScope->contains("b"));

	r = runGatherPass("const a=7\n const a=10");
	ASSERT_FALSE(r.ok());
	EXPECT_TRUE(m_pState->rootScope->contains("a"));
	EXPECT_EQ(ETYPE_SYMBOL_ALREADY_DEFINED_1, r.errors.front().type());

	//Nameless symbol test
	r = runGatherPass("function (a: int, b: int){a+b}");
	EXPECT_TRUE(r.ok());
}

/// <summary>
/// Tests 'gatherParameters' function.
/// </summary>
TEST_F(GatherPassTests, gatherParameters)
{
	//Detect collision with parameters
	auto r = runGatherPass("function test(a: int, b:int){const a = 3;}");
	EXPECT_SEM_ERROR(r);

	//Cannot shadow the name of a previous symbol
	r = runGatherPass("const a = 7\n function test(a: int){a * 2}");
	EXPECT_SEM_ERROR(r);

	//Shall not interfere with other tuple declarations
	r = runGatherPass(
		"function test(){\n"
		"  const a = 3;\n"
		"  var b:(a: int, b:int)\n"
		"}"
	);
	EXPECT_SEM_OK(r);
}


/// <summary>
/// Tests 'defaultToConst' function.
/// </summary>
TEST_F(GatherPassTests, defaultToConst)
{
	auto r = runGatherPass("function test(a: int, var b:int, const c:int){a*b*c}");
	ASSERT_SEM_OK(r);

	auto nodes = findNodes(r.result, [](Ref<AstNode> node) {
		return node->getType() == AST_DECLARATION;
	});

	ASSERT_EQ(3, nodes.size());

	EXPECT_TRUE(nodes[0]->hasFlag(ASTF_CONST));
	EXPECT_FALSE(nodes[0]->hasFlag(ASTF_VAR));

	EXPECT_FALSE(nodes[1]->hasFlag(ASTF_CONST));
	EXPECT_TRUE(nodes[1]->hasFlag(ASTF_VAR));

	EXPECT_TRUE(nodes[2]->hasFlag(ASTF_CONST));
	EXPECT_FALSE(nodes[2]->hasFlag(ASTF_VAR));
}
