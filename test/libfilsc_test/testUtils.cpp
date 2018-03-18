/// <summary>
/// Miscellaneous utilities to help testing FIL-S compiler.
/// </summary>

#include "libfilsc_test_pch.h"
#include "testUtils.h"
#include "semanticAnalysis.h"

using namespace std;

/// <summary>
/// Creates a token for testing, without file reference.
/// </summary>
/// <param name="code"></param>
/// <returns></returns>
LexToken testToken(const char* code)
{
    return LexToken(code, SourceFilePtr());
}

/// <summary>
/// Calls to parseScript, using a dummy file reference.
/// </summary>
/// <param name="code"></param>
/// <returns></returns>
ExprResult testParse(const char* code)
{
    return parseScript(code, SourceFilePtr());
}


/// <summary>
/// Checks an expression result object, and passes the error message to 'Google test'
/// if failed.
/// </summary>
/// <param name="res"></param>
/// <returns></returns>
::testing::AssertionResult checkExprOk(const ExprResult& res)
{
    if (res.ok())
        return ::testing::AssertionSuccess();
    else
        return ::testing::AssertionFailure() << res.errorDesc.what();
}

::testing::AssertionResult checkExprError(const ExprResult& res)
{
    if (res.error())
        return ::testing::AssertionSuccess();
    else
        return ::testing::AssertionFailure() << "Compilation error expected";
}

/// <summary>
/// Checks that a semantic analysis has succeded. If not, it  prints the error list
/// using 'google test' facilities.
/// </summary>
/// <param name="res"></param>
/// <returns></returns>
::testing::AssertionResult checkSemOk(const SemanticResult& res)
{
    if (res.ok())
        return ::testing::AssertionSuccess();
    else
    {
        auto result = ::testing::AssertionFailure() << "Found " << res.errors.size() << " errors:\n";

        for (auto error : res.errors)
            result << error.what() << "\n";

        return result;
    }
}

/// <summary>
/// Checks that the semantic analysis has errors.
/// </summary>
/// <param name="res"></param>
/// <returns></returns>
::testing::AssertionResult checkSemError(const SemanticResult& res)
{
    if (!res.ok())
        return ::testing::AssertionSuccess();
    else
        return ::testing::AssertionFailure() << "Semantic error expected";
}

/// <summary>
/// Checks that a parsing function consumes all input.
/// </summary>
/// <param name="code"></param>
/// <param name="parseFn"></param>
/// <returns></returns>
ExprResult checkAllParsed(const char* code, ParseFunction parseFn)
{
    auto r = parseFn(LexToken(code, SourceFilePtr()).next());

    if (r.ok() && !r.nextToken().eof())
        r = r.skip().getError(ETYPE_UNEXPECTED_TOKEN_2, r.nextText().c_str(), "<EOF>");
    return r;
};

/// <summary>
/// Performs a semantic analysis for a piece of code.
/// </summary>
/// <param name="code"></param>
/// <returns></returns>
SemanticResult semAnalysisCheck(const char* code)
{
    auto parseRes = parseScript(code, SourceFilePtr());

    if (parseRes.error())
        return SemanticResult(parseRes.errorDesc);
    else
        return semanticAnalysis(parseRes.result);
}

/// <summary>
/// Finds the nodes in the tree for which the predicate is true.
/// </summary>
/// <param name="root"></param>
/// <param name="predicate"></param>
/// <param name="result">The nodes found are appened to this vector</param>
void findNodes(Ref<AstNode> root, std::function<bool(Ref<AstNode>)> predicate, AstNodeList& result)
{
    if (predicate(root))
        result.push_back(root);

    for (auto child : root->children())
    {
        if (child.notNull())
            findNodes(child, predicate, result);
    }
}

/// <summary>
/// Finds the nodes in the tree for which the predicate is true
/// </summary>
/// <param name="root"></param>
/// <param name="predicate"></param>
/// <returns>Nodes are returned as a flat vector</returns>
AstNodeList findNodes(Ref<AstNode> root, std::function<bool(Ref<AstNode>)> predicate)
{
    AstNodeList result;

    findNodes(root, predicate, result);
    return result;
}

/// <summary>
/// Returns the first node which complies with the predicate.
/// </summary>
/// <param name="root"></param>
/// <param name="predicate"></param>
/// <returns></returns>
Ref<AstNode> findNode(Ref<AstNode> root, std::function<bool(Ref<AstNode>)> predicate)
{
    AstNodeList result;

    findNodes(root, predicate, result);
    if (result.empty())
        return Ref<AstNode>();
    else
        return result.front();
}

Ref<AstNode> findNode(Ref<AstNode> root, AstNodeTypes nodeType)
{
    return findNode(root, [nodeType](auto node) {
        return node->getType() == nodeType;
    });
}

Ref<AstNode> findNode(Ref<AstNode> root, const std::string& name)
{
    return findNode(root, [&name](auto node) {
        return node->getName() == name;
    });
}

/// <summary>Prints an AST tree.</summary>
/// <param name="node"></param>
/// <returns></returns>
std::string printAST(Ref<AstNode> node, int indentLevel)
{
    ostringstream	output;

    printAST(node, output, indentLevel);
    return output.str();
}

/// <summary>Prints out an AST tree to a stream</summary>
/// <param name="node"></param>
/// <param name="output"></param>
/// <param name="indentLevel"></param>
void printAST(Ref<AstNode> node, std::ostream& output, int indentLevel)
{
    output << string(indentLevel * 2, ' ');
    if (node.isNull())
        output << "[NULL]\n";
    else
    {
        output << astTypeToString(node->getType()) << "(";
        output << node->getName() << "," << node->getValue() << "): ";
        output << astTypeToString(node->getDataType()) << "\n";

        ++indentLevel;
        for (auto child : node->children())
            printAST(child, output, indentLevel);
    }
}
