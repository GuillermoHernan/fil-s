/// <summary>
/// Tests FIL-S semantic analyzer.
/// </summary>

#include "libfilsc_test_pch.h"
#include "semanticAnalysis_internal.h"
#include "semAnalysisState.h"

using namespace std;

/// <summary>
/// Tests 'semanticAnalysis' function.
/// </summary>
TEST(SemanticAnalysis, semanticAnalysis)
{
	const char* goodCode = "function add (a:int, b:int) {a+b}\n";

	const char* badCode = "function add (a:wrongType, b:wrongType) {a+b}\n";

	EXPECT_SEM_OK(semAnalysisCheck(goodCode));
	EXPECT_SEM_ERROR(semAnalysisCheck(badCode));
}


/// <summary>
/// Tests 'semInOrderWalk' function.
/// </summary>
TEST(SemanticAnalysis, semInOrderWalk)
{
	const char* testCode = "function max (a:int, b:int) {if (a>b) a else b}\n";
	const std::vector<AstNodeTypes> expected = {
		AST_TYPE_NAME,
		AST_DECLARATION,
		AST_TYPE_NAME,
		AST_DECLARATION,
		AST_TUPLE_DEF,
		AST_IDENTIFIER,
		AST_IDENTIFIER,
		AST_BINARYOP,
		AST_IDENTIFIER,
		AST_IDENTIFIER,
		AST_IF,
		AST_BLOCK,
		AST_FUNCTION,
		AST_SCRIPT
	};

	auto result = semAnalysisCheck(testCode);
	ASSERT_SEM_OK(result);

	vector<AstNodeTypes>	nodeTypes;
	SemAnalysisState		state;

	auto nodeFn = [&nodeTypes](Ref<AstNode> node, SemAnalysisState& state) {
		nodeTypes.push_back(node->getType());
		return SemanticResult(node);
	};

	result = semInOrderWalk(nodeFn, state, result.ast);
	ASSERT_SEM_OK(result);
	ASSERT_EQ(expected, nodeTypes);
}

/// <summary>
/// Tests 'semPreOrderWalk' function.
/// </summary>
TEST(SemanticAnalysis, semPreOrderWalk)
{
	const char* testCode = "function max (a:int, b:int) {if (a>b) a else b}\n";
	const std::vector<AstNodeTypes> expected = {
		AST_SCRIPT,
		AST_FUNCTION,
		AST_TUPLE_DEF,
		AST_DECLARATION,
		AST_TYPE_NAME,
		AST_DECLARATION,
		AST_TYPE_NAME,
		AST_BLOCK,
		AST_IF,
		AST_BINARYOP,
		AST_IDENTIFIER,
		AST_IDENTIFIER,
		AST_IDENTIFIER,
		AST_IDENTIFIER,
	};

	auto result = semAnalysisCheck(testCode);
	ASSERT_SEM_OK(result);

	vector<AstNodeTypes>	nodeTypes;
	SemAnalysisState		state;

	auto nodeFn = [&nodeTypes](Ref<AstNode> node, SemAnalysisState& state) {
		nodeTypes.push_back(node->getType());
		return SemanticResult(node);
	};

	result = semPreOrderWalk(nodeFn, state, result.ast);
	ASSERT_SEM_OK(result);
	ASSERT_EQ(expected, nodeTypes);
}
