/// <summary>
/// Tests FIL-S parser.
/// </summary>

#include "libfilsc_test_pch.h"
#include "parser_internal.h"
#include "compileError.h"

/// <summary>
/// Tests 'parseScript' function.
/// </summary>
TEST(Parser, parseScript)
{
    const char* goodCode = "function add (a:int, b:int) {a+b};;\n;\n;\n;\n";

    const char* badCode = "funcion add (a:int, b:int) {a+b}\n";

    auto result = parseScript(goodCode, SourceFilePtr());
    EXPECT_STREQ("", result.errorDesc.what());
    ASSERT_TRUE(result.ok());
    EXPECT_EQ(AST_SCRIPT, result.result->getType());

    result = parseScript(badCode, SourceFilePtr());
    EXPECT_FALSE(result.ok());

    result = parseScript("", SourceFilePtr());
    EXPECT_TRUE(result.ok());
    EXPECT_FALSE(result.result->childExists(0));
}

/// <summary>
/// Tests 'isAssignment' function, which checks if an operator is an assignment operator.
/// </summary>
TEST(Parser, isAssignment)
{
    const char * assignmentOps = "= >>= <<= += -= *= /= %= &= |= ^=";
    const char * otherOps = ">> << + - * / % < > <= >= ";

    LexToken	tok(assignmentOps, SourceFilePtr());

    for (tok = tok.next(); !tok.eof(); tok = tok.next())
        EXPECT_TRUE(isAssignment(tok));

    tok = LexToken(otherOps, SourceFilePtr());

    for (tok = tok.next(); !tok.eof(); tok = tok.next())
        EXPECT_FALSE(isAssignment(tok));
}


/// <summary>
/// Tests 'isBinaryOp' function, which checks if an operator is a binary operator.
/// </summary>
TEST(Parser, isBinaryOp)
{
    const char * binaryOps = ">>> >> << ** + - * / % & | && || ^ < > >= <= == != ";
    const char * otherOps = "-= += ~ ! ++ --";

    LexToken	tok(binaryOps, SourceFilePtr());

    for (tok = tok.next(); !tok.eof(); tok = tok.next())
        EXPECT_TRUE(isBinaryOp(tok));

    tok = LexToken(otherOps, SourceFilePtr());

    for (tok = tok.next(); !tok.eof(); tok = tok.next())
        EXPECT_FALSE(isBinaryOp(tok));
}


/// <summary>
/// Tests 'isPrefixOp' function, which checks if an operator is a prefix operator.
/// </summary>
TEST(Parser, isPrefixOp)
{
    const char * prefixOps = "! ~ + - ++ --";
    const char * otherOps = ">>> >> << ** * / % & | && || ^ < > >= <= == != ";

    LexToken	tok(prefixOps, SourceFilePtr());

    for (tok = tok.next(); !tok.eof(); tok = tok.next())
        EXPECT_TRUE(isPrefixOp(tok));

    tok = LexToken(otherOps, SourceFilePtr());

    for (tok = tok.next(); !tok.eof(); tok = tok.next())
        EXPECT_FALSE(isPrefixOp(tok));
}


/// <summary>
/// Tests 'isPostfixOp' function, which checks if an operator is a postfix operator.
/// </summary>
TEST(Parser, isPostfixOp)
{
    const char * postFixOps = "++ --";
    const char * otherOps = "! | ~ >>> >> << ** * / % & | && || ^ < > >= <= == != ";

    LexToken	tok(postFixOps, SourceFilePtr());

    for (tok = tok.next(); !tok.eof(); tok = tok.next())
        EXPECT_TRUE(isPostfixOp(tok));

    tok = LexToken(otherOps, SourceFilePtr());

    for (tok = tok.next(); !tok.eof(); tok = tok.next())
        EXPECT_FALSE(isPostfixOp(tok));
}

/// <summary>
/// Tests for 'parseList' function. This function is used to parse nodes which are build from 
/// a list of other nodes.
/// </summary>
TEST(Parser, parseList)
{
    const char * fullList = "(1,2,3,4,5,6,7)";
    const char * sepList = "1,2,3,4,5,6,7)";
    const char * nosepList = "{1 2 3 4 5 6 7}";
    const char * simpleList = "1 2 3 4 5 6 7)";

    auto parseList_ = [](const char* code, const char* beginTok, const char* endTok, const char* separator)
    {
        LexToken	tok(code, SourceFilePtr());
        return parseList(tok.next(), parseLiteral, beginTok, endTok, separator);
    };

    EXPECT_PARSE_OK(parseList_(fullList, "(", ")", ","));
    EXPECT_PARSE_ERROR(parseList_(fullList, "", ")", ","));
    EXPECT_PARSE_ERROR(parseList_(fullList, "", ")", ""));
    EXPECT_PARSE_ERROR(parseList_(fullList, "(", ")", ""));

    EXPECT_PARSE_ERROR(parseList_(sepList, "(", ")", ","));
    EXPECT_PARSE_OK(parseList_(sepList, "", ")", ","));
    EXPECT_PARSE_ERROR(parseList_(sepList, "", ")", ""));
    EXPECT_PARSE_ERROR(parseList_(sepList, "(", ")", ""));

    EXPECT_PARSE_ERROR(parseList_(nosepList, "{", "}", ","));
    EXPECT_PARSE_ERROR(parseList_(nosepList, "", "}", ","));
    EXPECT_PARSE_ERROR(parseList_(nosepList, "", "}", ""));
    EXPECT_PARSE_OK(parseList_(nosepList, "{", "}", ""));
    EXPECT_PARSE_ERROR(parseList_(nosepList, "(", ")", ""));

    EXPECT_PARSE_ERROR(parseList_(simpleList, "(", "#", ","));
    EXPECT_PARSE_ERROR(parseList_(simpleList, "", ")", ","));
    EXPECT_PARSE_OK(parseList_(simpleList, "", ")", ""));
    EXPECT_PARSE_ERROR(parseList_(simpleList, "(", ")", ""));
}

/// <summary>
/// Tests for 'parseBlock' function.
/// </summary>
/// <param name=""></param>
/// <param name=""></param>
/// <returns></returns>
TEST(Parser, parseBlock)
{
    auto parseBlock_ = [](const char* code)
    {
        return parseBlock(LexToken(code, SourceFilePtr()).next());
    };

    const char * block1 =
        "{\n"
        "  const a = 7 + 6\n"
        "  const b = 4 * 5\n"
        "  b - a\n"
        "}\n";

    const char * withSemicolons =
        "{\n"
        "  const a = 7 + 6;\n"
        "  const b = 4 * 5;;;\n"
        "  b - a;\n"
        "}\n";

    const char * errorInExp = "{10 - 4 , 3}";
    const char * notClosed = "{10 + 4 + 3";
    const char * wrongCloseToken = "{10 + 4 + 3)";
    const char * notBlock = "(10 + 4 + 3)";

    EXPECT_PARSE_OK(parseBlock_(block1));
    EXPECT_PARSE_OK(parseBlock_(withSemicolons));

    EXPECT_PARSE_ERROR(parseBlock_(errorInExp));
    EXPECT_PARSE_ERROR(parseBlock_("{%n}"));
    EXPECT_PARSE_ERROR(parseBlock_(notClosed));
    EXPECT_PARSE_ERROR(parseBlock_(wrongCloseToken));
    EXPECT_PARSE_ERROR(parseBlock_(notBlock));
}

/// <summary>
/// Tests 'parseDeclaration' function.
/// Also tests 'parseTypeSpecifier' and 'parseTypeDescriptor' functions.
/// </summary>
TEST(Parser, parseDeclaration)
{
    auto parseDeclaration_ = [](const char* code)
    {
        return checkAllParsed(code, parseDeclaration);
    };

    EXPECT_PARSE_OK(parseDeclaration_("standAlone"));
    EXPECT_PARSE_OK(parseDeclaration_("typed:int"));
    EXPECT_PARSE_OK(parseDeclaration_("initialized = a"));
    EXPECT_PARSE_OK(parseDeclaration_("full:int = 3"));
    EXPECT_PARSE_OK(parseDeclaration_("tuple1:(int, int, float)"));
    EXPECT_PARSE_OK(parseDeclaration_("array1[10]:int"));
    EXPECT_PARSE_OK(parseDeclaration_("array2[5]:(int, int, float)"));

    EXPECT_PARSE_ERROR(parseDeclaration_("var full:int = a"));
    EXPECT_PARSE_ERROR(parseDeclaration_("if = 3"));
    EXPECT_PARSE_ERROR(parseDeclaration_("const: int"));

    auto r = parseDeclaration_("typed:int");
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(2, r.result->children().size());
    ASSERT_TRUE(r.result->children()[0].notNull());
    EXPECT_EQ(AST_TYPE_NAME, r.result->children()[0]->getType());

    r = parseDeclaration_("tuple1:(int, int, float)");
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(2, r.result->children().size());
    ASSERT_TRUE(r.result->children()[0].notNull());
    EXPECT_EQ(AST_TUPLE_DEF, r.result->children()[0]->getType());
}

/// <summary>
/// Tests 'parseArrayDeclaration' function.
/// </summary>
/// <param name=""></param>
/// <param name=""></param>
/// <returns></returns>
TEST(Parser, parseArrayDeclaration)
{
    auto parseArrayDeclaration_ = [](const char* code)
    {
        return checkAllParsed(code, parseArrayDeclaration);
    };

    EXPECT_PARSE_OK(parseArrayDeclaration_("[10]:int"));
    EXPECT_PARSE_OK(parseArrayDeclaration_("[5][10]:int"));
    EXPECT_PARSE_OK(parseArrayDeclaration_("[5][10]:(int,bool)"));

    EXPECT_PARSE_ERROR(parseArrayDeclaration_("35"));
    EXPECT_PARSE_ERROR(parseArrayDeclaration_(":[10]:int"));
    EXPECT_PARSE_ERROR(parseArrayDeclaration_("[]:int"));
    EXPECT_PARSE_ERROR(parseArrayDeclaration_("[2]"));
    EXPECT_PARSE_ERROR(parseArrayDeclaration_("[2:int"));
    EXPECT_PARSE_ERROR(parseArrayDeclaration_("[2]int"));
    EXPECT_PARSE_ERROR(parseArrayDeclaration_("[2]::int"));

    auto r = parseArrayDeclaration_("[5][10]:int");
    ASSERT_TRUE(r.ok());

    ASSERT_EQ(AST_ARRAY_DECL, r.result->getType());
    
    auto arrayNodes = findNodes(r.result, [](Ref<AstNode> n) {
        return n->getType() == AST_ARRAY_DECL;
    });

    EXPECT_EQ(2, arrayNodes.size());

    ASSERT_EQ(2, r.result->childCount());
    EXPECT_EQ(AST_ARRAY_DECL, r.result->child(0)->getType());
    EXPECT_EQ(AST_INTEGER, r.result->child(1)->getType());
    EXPECT_STREQ("5", r.result->child(1)->getValue().c_str());
}

/// <summary>
/// Tests 'parseConst' and 'parseVar' functions.
/// </summary>
TEST(Parser, parseVarConst)
{
    auto parseVar_ = [](const char* code)
    {
        return checkAllParsed(code, parseVar);
    };
    auto parseConst_ = [](const char* code)
    {
        return checkAllParsed(code, parseConst);
    };

    EXPECT_PARSE_OK(parseVar_("var a"));
    EXPECT_PARSE_OK(parseVar_("var a: int = 7"));
    EXPECT_PARSE_OK(parseConst_("const a"));
    EXPECT_PARSE_OK(parseConst_("const a: int = 7"));

    EXPECT_PARSE_ERROR(parseConst_("var a"));
    EXPECT_PARSE_ERROR(parseConst_("var a: int = 7"));
    EXPECT_PARSE_ERROR(parseVar_("const a"));
    EXPECT_PARSE_ERROR(parseVar_("const a: int = 7"));

    EXPECT_PARSE_ERROR(parseVar_("a: int = 7"));
    EXPECT_PARSE_ERROR(parseConst_("a: int = 7"));
}

/// <summary>
/// Tests 'parseTupleDef' and 'parseTupleDefItem' functions
/// </summary>
TEST(Parser, parseTupleDef)
{
    auto parseTupleDef_ = [](const char* code)
    {
        return checkAllParsed(code, parseTupleDef);
    };

    EXPECT_PARSE_OK(parseTupleDef_("(int, int, int)"));
    EXPECT_PARSE_OK(parseTupleDef_("(float)"));
    EXPECT_PARSE_OK(parseTupleDef_("()"));
    EXPECT_PARSE_OK(parseTupleDef_("(a:int, b:float)"));
    EXPECT_PARSE_OK(parseTupleDef_("(const a:int, b:float, var c:string)"));
    EXPECT_PARSE_OK(parseTupleDef_("(int, int, (float, otherType))"));

    EXPECT_PARSE_ERROR(parseTupleDef_("(1, 2, 3)"));
    EXPECT_PARSE_ERROR(parseTupleDef_("(int, int"));
    EXPECT_PARSE_ERROR(parseTupleDef_("u(int, int)"));
    EXPECT_PARSE_ERROR(parseTupleDef_("(int, int; int"));

    auto r = parseTupleDef_("(int, bool)");
    auto node = r.result;

    ASSERT_PARSE_OK(r);
    ASSERT_EQ(2, node->childCount());
    ASSERT_EQ(AST_DECLARATION, node->child(0)->getType());
    ASSERT_EQ(AST_DECLARATION, node->child(1)->getType());
}


/// <summary>
/// Tests 'parseIf' function
/// </summary>
TEST(Parser, parseIf)
{
    auto parseIf_ = [](const char* code)
    {
        return checkAllParsed(code, parseIf);
    };

    EXPECT_PARSE_OK(parseIf_("if (a) b"));
    EXPECT_PARSE_OK(parseIf_("if (a) b;"));
    EXPECT_PARSE_OK(parseIf_("if (a) b else c"));
    EXPECT_PARSE_OK(parseIf_("if (a) b; else c"));
    EXPECT_PARSE_OK(parseIf_("if (a) {b; c; d;} else x"));

    EXPECT_PARSE_ERROR(parseIf_("isf (a) b"));
    EXPECT_PARSE_ERROR(parseIf_("if a) b"));
    EXPECT_PARSE_ERROR(parseIf_("if (a b"));
    EXPECT_PARSE_ERROR(parseIf_("if (a b else c"));
    EXPECT_PARSE_ERROR(parseIf_("if (a) b c"));
    EXPECT_PARSE_ERROR(parseIf_("if (a) b;; else c"));

    auto result = parseIf_("if (a>0) a else -a");
    EXPECT_PARSE_OK(result);

    auto ifNode = result.result;

    EXPECT_EQ(AST_IF, ifNode->getType());
    ASSERT_EQ(3, ifNode->children().size());
    ASSERT_TRUE(ifNode->childExists(0));
    ASSERT_TRUE(ifNode->childExists(1));
    ASSERT_TRUE(ifNode->childExists(2));

    EXPECT_EQ(AST_BINARYOP, ifNode->children()[0]->getType());
    EXPECT_EQ(AST_IDENTIFIER, ifNode->children()[1]->getType());
    EXPECT_EQ(AST_PREFIXOP, ifNode->children()[2]->getType());
}


/// <summary>
/// Tests 'parseExpression' function
/// </summary>
TEST(Parser, parseExpression)
{
    auto parseExpression_ = [](const char* code)
    {
        return checkAllParsed(code, parseExpression);
    };

    EXPECT_PARSE_OK(parseExpression_("a*=b"));
    EXPECT_PARSE_OK(parseExpression_("a+b"));
    EXPECT_PARSE_OK(parseExpression_("-a"));
    EXPECT_PARSE_OK(parseExpression_("a++"));
    EXPECT_PARSE_OK(parseExpression_("(hello)"));
    EXPECT_PARSE_OK(parseExpression_("if (a) b else c"));

    EXPECT_PARSE_ERROR(parseExpression_("const a = 7"));
    EXPECT_PARSE_ERROR(parseExpression_("var a = 7"));
}

/// <summary>
/// Tests 'parseTerm' function
/// </summary>
TEST(Parser, parseTerm)
{
    auto parseTerm_ = [](const char* code)
    {
        return checkAllParsed(code, parseTerm);
    };

    EXPECT_PARSE_OK(parseTerm_("a"));
    EXPECT_PARSE_OK(parseTerm_("8"));
    EXPECT_PARSE_OK(parseTerm_("\"test\""));
    EXPECT_PARSE_OK(parseTerm_("a.b"));
    EXPECT_PARSE_OK(parseTerm_("(1,2,3)"));
    EXPECT_PARSE_OK(parseTerm_("(a-b)"));
    EXPECT_PARSE_OK(parseTerm_("{a-b}"));
    EXPECT_PARSE_OK(parseTerm_("if (a) b else c"));
    EXPECT_PARSE_OK(parseTerm_("a++"));

    EXPECT_PARSE_ERROR(parseTerm_("a=b"));
    EXPECT_PARSE_ERROR(parseTerm_("a*=b"));
    EXPECT_PARSE_ERROR(parseTerm_("a+b"));
    EXPECT_PARSE_ERROR(parseTerm_("-a"));
}

/// <summary>
/// Tests 'parseAssignment' function
/// </summary>
TEST(Parser, parseAssignment)
{
    auto parseAssignment_ = [](const char* code)
    {
        return checkAllParsed(code, parseAssignment);
    };

    EXPECT_PARSE_OK(parseAssignment_("a = b"));
    EXPECT_PARSE_OK(parseAssignment_("x *= 9"));
    EXPECT_PARSE_OK(parseAssignment_("r %= 9"));

    EXPECT_PARSE_ERROR(parseAssignment_("r %= 9 c"));
    EXPECT_PARSE_ERROR(parseAssignment_("x"));
    EXPECT_PARSE_ERROR(parseAssignment_("x + y"));

    auto result = parseAssignment_("a = b");
    EXPECT_EQ(AST_ASSIGNMENT, result.result->getType());
}

/// <summary>
/// Tests for 'parseLeftExpr' function
/// </summary>
TEST(Parser, parseLeftExpr)
{
    auto parseLeftExpr_ = [](const char* code)
    {
        return checkAllParsed(code, parseLeftExpr);
    };

    EXPECT_PARSE_OK(parseLeftExpr_("a++"));
    EXPECT_PARSE_OK(parseLeftExpr_("a.c"));
    //EXPECT_PARSE_OK(parseLeftExpr_("a.c(1,2)"));
    EXPECT_PARSE_OK(parseLeftExpr_("fn(7)"));
    EXPECT_PARSE_OK(parseLeftExpr_("x"));
    EXPECT_PARSE_OK(parseLeftExpr_("2"));
    EXPECT_PARSE_OK(parseLeftExpr_("(x+6)"));

    EXPECT_PARSE_ERROR(parseLeftExpr_("r = 9"));
    EXPECT_PARSE_ERROR(parseLeftExpr_("3 + 3"));
}

/// <summary>
/// Tests for 'parseBinaryExpr' function
/// </summary>
TEST(Parser, parseBinaryExpr)
{
    auto parseBinaryExpr_ = [](const char* code)
    {
        return checkAllParsed(code, parseBinaryExpr);
    };

    EXPECT_PARSE_OK(parseBinaryExpr_("a + 9"));
    EXPECT_PARSE_OK(parseBinaryExpr_("a + 9 + 7"));
    EXPECT_PARSE_OK(parseBinaryExpr_("a + b.c + 7"));
    EXPECT_PARSE_OK(parseBinaryExpr_("a + 9 + (7-x)"));
    EXPECT_PARSE_OK(parseBinaryExpr_("a > 0"));

    EXPECT_PARSE_ERROR(parseBinaryExpr_("r = 9"));
    EXPECT_PARSE_ERROR(parseBinaryExpr_("-n"));
    EXPECT_PARSE_ERROR(parseBinaryExpr_("a + b - c"));

    auto result = parseBinaryExpr_("a + b - c");
    EXPECT_EQ(ETYPE_INVALID_EXP_CHAIN, result.errorDesc.type());
    EXPECT_EQ(7, result.errorDesc.position().column());

    result = parseBinaryExpr_("a > 0");
    auto node = result.result;

    ASSERT_PARSE_OK(result);
    EXPECT_EQ(AST_BINARYOP, node->getType());

    result = parseBinaryExpr_("a + 9 + 7");
    node = result.result;

    ASSERT_PARSE_OK(result);
    EXPECT_EQ(AST_BINARYOP, node->getType());
    ASSERT_TRUE(node->childExists(0));
    EXPECT_TRUE(node->childExists(1));
    EXPECT_FALSE(node->childExists(2));
    EXPECT_EQ(AST_BINARYOP, node->children()[0]->getType());
    EXPECT_STREQ("a", node->children()[0]->children()[0]->getName().c_str());
    EXPECT_STREQ("9", node->children()[0]->children()[1]->getValue().c_str());
    EXPECT_STREQ("7", node->children()[1]->getValue().c_str());
}

/// <summary>
/// Tests for 'parsePrefixExpr' function
/// </summary>
TEST(Parser, parsePrefixExpr)
{
    auto parsePrefixExpr_ = [](const char* code)
    {
        return checkAllParsed(code, parsePrefixExpr);
    };

    EXPECT_PARSE_OK(parsePrefixExpr_("-1"));
    EXPECT_PARSE_OK(parsePrefixExpr_("-a"));
    EXPECT_PARSE_OK(parsePrefixExpr_("+u"));
    EXPECT_PARSE_OK(parsePrefixExpr_("!a"));
    EXPECT_PARSE_OK(parsePrefixExpr_("~r"));
    EXPECT_PARSE_OK(parsePrefixExpr_("++i"));
    EXPECT_PARSE_OK(parsePrefixExpr_("--j"));

    EXPECT_PARSE_ERROR(parsePrefixExpr_("-~c"));
    EXPECT_PARSE_ERROR(parsePrefixExpr_("r + 9"));
    EXPECT_PARSE_ERROR(parsePrefixExpr_("i++"));
    EXPECT_PARSE_ERROR(parsePrefixExpr_("++\ni"));
}

/// <summary>
/// Tests for 'parsePostfixExpr', 'parsePostfixOperator' and 'parseMemberAccess' functions
/// </summary>
TEST(Parser, parsePostfixExpr)
{
    auto parsePostfixExpr_ = [](const char* code)
    {
        return checkAllParsed(code, parsePostfixExpr);
    };

    EXPECT_PARSE_OK(parsePostfixExpr_("i++"));
    EXPECT_PARSE_OK(parsePostfixExpr_("u--"));
    EXPECT_PARSE_OK(parsePostfixExpr_("a.b"));
    EXPECT_PARSE_OK(parsePostfixExpr_("a.b.c.d.e"));
    EXPECT_PARSE_OK(parsePostfixExpr_("a(3,n)"));
    EXPECT_PARSE_OK(parsePostfixExpr_("a.b(3).c.d(9,(7,a)).e"));

    EXPECT_PARSE_ERROR(parsePostfixExpr_("--c"));
    EXPECT_PARSE_ERROR(parsePostfixExpr_("i++ ++"));
    EXPECT_PARSE_ERROR(parsePostfixExpr_("c+4"));
    EXPECT_PARSE_ERROR(parsePostfixExpr_("i\n++"));
}

/// <summary>
/// Tests for 'parseCallExpr' function
/// </summary>
TEST(Parser, parseCallExpr)
{
    auto parseCallExpr_ = [](const char* code)
    {
        //It does not call 'parseCallExpr' because it needs the previous expression
        //(the expression which yields the function to be called)
        return checkAllParsed(code, parsePostfixExpr);
    };

    EXPECT_PARSE_OK(parseCallExpr_("fn(b,c)"));
    EXPECT_PARSE_OK(parseCallExpr_("fn(\nb,c)"));
    EXPECT_PARSE_OK(parseCallExpr_("fn(1)"));
    EXPECT_PARSE_OK(parseCallExpr_("fn()"));
    EXPECT_PARSE_OK(parseCallExpr_("obj.fn(1,2)"));
    EXPECT_PARSE_OK(parseCallExpr_("obj.fn(1,2)(3)(4)"));

    EXPECT_PARSE_ERROR(parseCallExpr_("fn 3"));
    EXPECT_PARSE_ERROR(parseCallExpr_("fn (3"));
    EXPECT_PARSE_ERROR(parseCallExpr_("fn 3)"));
    EXPECT_PARSE_ERROR(parseCallExpr_("fn\n(b,c)"));
}

/// <summary>
/// Tests for 'parseLiteral' function
/// </summary>
TEST(Parser, parseLiteral)
{
    auto parseLiteral_ = [](const char* code)
    {
        return checkAllParsed(code, parseLiteral);
    };

    EXPECT_PARSE_OK(parseLiteral_("1"));
    EXPECT_PARSE_OK(parseLiteral_("1000000"));
    EXPECT_PARSE_OK(parseLiteral_("46.9"));
    EXPECT_PARSE_OK(parseLiteral_("\"test\""));

    EXPECT_PARSE_ERROR(parseLiteral_("1 3"));
    EXPECT_PARSE_ERROR(parseLiteral_("x"));
}

/// <summary>
/// Tests for 'parseTuple' function
/// </summary>
TEST(Parser, parseTuple)
{
    auto parseTuple_ = [](const char* code)
    {
        return checkAllParsed(code, parseTuple);
    };

    EXPECT_PARSE_OK(parseTuple_("()"));
    EXPECT_PARSE_OK(parseTuple_("(1)"));
    EXPECT_PARSE_OK(parseTuple_("(1,2,a,c)"));
    EXPECT_PARSE_OK(parseTuple_("(a, (b+c, fn(r)), 8.96)"));

    EXPECT_PARSE_ERROR(parseTuple_("1,3"));
    EXPECT_PARSE_ERROR(parseTuple_("(1,3"));
    EXPECT_PARSE_ERROR(parseTuple_("a"));
    EXPECT_PARSE_ERROR(parseTuple_("3"));
}

/// <summary>
/// Tests for 'parseIdentifier' function
/// </summary>
TEST(Parser, parseIdentifier)
{
    auto parseIdentifier_ = [](const char* code)
    {
        return checkAllParsed(code, parseIdentifier);
    };

    EXPECT_PARSE_OK(parseIdentifier_("a"));
    EXPECT_PARSE_OK(parseIdentifier_("Xe45"));
    EXPECT_PARSE_OK(parseIdentifier_("CamelCase"));
    EXPECT_PARSE_OK(parseIdentifier_("a2"));
    EXPECT_PARSE_OK(parseIdentifier_("a'"));
    EXPECT_PARSE_OK(parseIdentifier_("a''"));
    EXPECT_PARSE_OK(parseIdentifier_("_a_"));

    EXPECT_PARSE_ERROR(parseIdentifier_("1a"));
    EXPECT_PARSE_ERROR(parseIdentifier_("for"));
    EXPECT_PARSE_ERROR(parseIdentifier_("if"));
    EXPECT_PARSE_ERROR(parseIdentifier_("else"));

    auto r = parseIdentifier_("test1'");

    EXPECT_EQ(AST_IDENTIFIER, r.result->getType());
    EXPECT_STREQ("test1'", r.result->getName().c_str());
}

/// <summary>
/// Tests for 'parseFunctionDef' function
/// </summary>
TEST(Parser, parseFunctionDef)
{
    auto parseFunctionDef_ = [](const char* code)
    {
        return checkAllParsed(code, parseFunctionDef);
    };

    EXPECT_PARSE_OK(parseFunctionDef_("function (x:int):int {x*2}"));
    EXPECT_PARSE_OK(parseFunctionDef_("function (x:int):int x*2"));
    EXPECT_PARSE_OK(parseFunctionDef_("function testFn(x:int):int {x*2}"));
    EXPECT_PARSE_OK(parseFunctionDef_(
        "function testFn(x:int)\n"
        "{\n"
        "  print(x);\n"
        "}"
    ));
    EXPECT_PARSE_OK(parseFunctionDef_(
        "function testFn (a:float, b:float): float {(a + b) /2}"
    ));
    EXPECT_PARSE_OK(parseFunctionDef_(
        "function[C] testFn (a:int, b:int):int"
    ));

    //This is an error, but it is not detected by the parser.
    EXPECT_PARSE_OK(parseFunctionDef_(
        "function[C] testFn (a:int, b:int):int {(a + b) /2}"
    ));

    EXPECT_PARSE_ERROR(parseFunctionDef_("(x:int):int {x*2}"));
    EXPECT_PARSE_ERROR(parseFunctionDef_("function (x:int):int"));

    auto parseR = parseFunctionDef_("function testFn (a:float, b:float): float {(a + b) /2}");
    auto r = parseR.result;

    EXPECT_EQ(AST_FUNCTION, r->getType());
    EXPECT_STREQ("testFn", r->getName().c_str());

    auto& children = r->children();
    ASSERT_EQ(3, children.size());
    EXPECT_EQ(AST_TUPLE_DEF, children[0]->getType());
    EXPECT_EQ(AST_TYPE_NAME, children[1]->getType());
    EXPECT_EQ(AST_BLOCK, children[2]->getType());

    EXPECT_EQ(2, children[0]->children().size());
}

/// <summary>
/// Tests for 'parseFunctionType' function
/// </summary>
TEST(Parser, parseFunctionType)
{
    auto parseFunctionType_ = [](const char* code)
    {
        return checkAllParsed(code, parseFunctionType);
    };

    EXPECT_PARSE_OK(parseFunctionType_("function (x:int):int"));
    EXPECT_PARSE_OK(parseFunctionType_("function (x:int)"));
    EXPECT_PARSE_ERROR(parseFunctionType_("function (x:int):int {x*2}"));
    EXPECT_PARSE_ERROR(parseFunctionType_("function (x:int):int x*2"));
    EXPECT_PARSE_ERROR(parseFunctionType_("function name(x:int):int"));

    auto parseR = parseFunctionType_("function (a:float, b:float): float");
    auto r = parseR.result;

    EXPECT_EQ(AST_FUNCTION_TYPE, r->getType());
    EXPECT_STREQ("", r->getName().c_str());

    auto& children = r->children();
    ASSERT_EQ(2, children.size());
    EXPECT_EQ(AST_TUPLE_DEF, children[0]->getType());
    EXPECT_EQ(AST_TYPE_NAME, children[1]->getType());

    EXPECT_EQ(2, children[0]->children().size());
}

/// <summary>
/// Tests for 'parseInputType' function
/// </summary>
TEST(Parser, parseInputType)
{
    auto parseInputType_ = [](const char* code)
    {
        return checkAllParsed(code, parseInputType);
    };

    EXPECT_PARSE_ERROR(parseInputType_("input (x:int):int"));
    EXPECT_PARSE_OK(parseInputType_("input (x:int)"));
    EXPECT_PARSE_OK(parseInputType_("input ()"));
    EXPECT_PARSE_ERROR(parseInputType_("input (x:int){x*2}"));
    EXPECT_PARSE_ERROR(parseInputType_("input name(x:int)"));

    auto parseR = parseInputType_("input (a:float, b:float)");
    ASSERT_PARSE_OK(parseR);
    auto r = parseR.result;

    EXPECT_EQ(AST_MESSAGE_TYPE, r->getType());
    EXPECT_STREQ("", r->getName().c_str());

    auto& children = r->children();
    ASSERT_EQ(1, children.size());
    EXPECT_EQ(AST_TUPLE_DEF, children[0]->getType());

    EXPECT_EQ(2, children[0]->children().size());
}


/// <summary>
/// Tests for 'parsePrimaryExpr' function
/// </summary>
TEST(Parser, parsePrimaryExpr)
{
    auto parsePrimaryExpr_ = [](const char* code)
    {
        return checkAllParsed(code, parsePrimaryExpr);
    };

    EXPECT_PARSE_OK(parsePrimaryExpr_("0x33"));
    EXPECT_PARSE_OK(parsePrimaryExpr_("test"));
    EXPECT_PARSE_OK(parsePrimaryExpr_("(a+b)"));
    EXPECT_PARSE_OK(parsePrimaryExpr_("{c=a+b; c*2;}"));
    EXPECT_PARSE_OK(parsePrimaryExpr_("(1,2,3)"));

    EXPECT_PARSE_ERROR(parsePrimaryExpr_("9+4"));
}

/// <summary>
/// Tests for 'parseTypedef' function
/// </summary>
TEST(Parser, parseTypedef)
{
    auto parseTypedef_ = [](const char* code)
    {
        return checkAllParsed(code, parseTypedef);
    };

    EXPECT_PARSE_OK(parseTypedef_("type integer is int"));
    EXPECT_PARSE_OK(parseTypedef_("type vec2f is (float, float)"));
    EXPECT_PARSE_OK(parseTypedef_("type test is (a:float, b:float=7.0, c=\"test\")"));

    EXPECT_PARSE_ERROR(parseTypedef_("vec2f is (float, float)"));
    EXPECT_PARSE_ERROR(parseTypedef_("type vec2f (float, float)"));
    EXPECT_PARSE_ERROR(parseTypedef_("type integer is 17"));
}

/// <summary>
/// Tests for 'parseStruct' function
/// </summary>
TEST(Parser, parseStruct)
{
    auto parseStruct_ = [](const char* code)
    {
        return checkAllParsed(code, parseStruct);
    };

    EXPECT_PARSE_OK(parseStruct_("struct[C] test(a:int, b:int)"));

    EXPECT_PARSE_ERROR(parseStruct_("struct test(a:int, b:int)"));
    EXPECT_PARSE_ERROR(parseStruct_("type test is (a:int, b:int)"));
}

/// <summary>
/// Tests for 'parseStatementSeparator' function.
/// </summary>
/// <param name=""></param>
/// <param name=""></param>
/// <returns></returns>
TEST(Parser, parseStatementSeparator)
{
    const char* goodCode =
        "const a = 5\n"
        "const b = 6; const c = 7\n";

    const char* badCode =
        "const a = 5\n"
        "const b = 6  const c = 7\n";

    EXPECT_PARSE_OK(parseScript(goodCode, SourceFilePtr()));

    auto r = parseScript(badCode, SourceFilePtr());
    ASSERT_PARSE_ERROR(r);

    auto r2 = parseStatementSeparator(r);
    ASSERT_PARSE_ERROR(r2);

    auto pos1 = r.errorDesc.position();
    auto pos2 = r2.errorDesc.position();
    EXPECT_TRUE(pos1 == pos2);
}

/// <summary>
/// Tests for 'parseReturn' function.
/// </summary>
/// <param name=""></param>
/// <param name=""></param>
/// <returns></returns>
TEST(Parser, parseReturn)
{
    auto parseReturn_ = [](const char* code)
    {
        return checkAllParsed(code, parseReturn);
    };

    EXPECT_PARSE_OK(parseReturn_("return a+5"));
    EXPECT_PARSE_OK(parseReturn_("return\n"));
    EXPECT_PARSE_OK(parseReturn_("(9*n) + (x*y*z)"));

    EXPECT_PARSE_ERROR(parseReturn_("const a = 6"));
    EXPECT_PARSE_ERROR(parseReturn_("return const a = 6"));
}


/// <summary>
/// Tests for 'markAsParameters' function
/// </summary>
TEST(Parser, markAsParameters)
{
    auto parseFunctionDef_ = [](const char* code)
    {
        return checkAllParsed(code, parseFunctionDef);
    };

    auto r = parseFunctionDef_("function (x:int, y:int):int {\n"
        "const z = (x*y) + 3;\n"
        "z + (y / 2);\n"
        "}");

    ASSERT_PARSE_OK(r);

    auto nodes = findNodes(r.result, [](Ref<AstNode> node) {
        return node->getType() == AST_DECLARATION;
    });

    ASSERT_EQ(3, nodes.size());
    EXPECT_TRUE(nodes[0]->hasFlag(ASTF_FUNCTION_PARAMETER));
    EXPECT_TRUE(nodes[1]->hasFlag(ASTF_FUNCTION_PARAMETER));
    EXPECT_FALSE(nodes[2]->hasFlag(ASTF_FUNCTION_PARAMETER));
    EXPECT_STREQ("z", nodes[2]->getName().c_str());
}

/// <summary>
/// Tests for 'parseActorDef' function.
/// Also tests 'parseInputMsg', 'parseOutputMsg' functions.
/// </summary>
TEST(Parser, parseActorDef)
{
    auto parseActorDef_ = [](const char* code)
    {
        return parseActorDef(LexToken(code, SourceFilePtr()).next());
    };

    EXPECT_PARSE_OK(parseActorDef_(
        "actor Test1{\n"
        "  output o1(int)\n"
        "  input i1(a: int, b: int){\n"
        "    o1(a+b);\n"
        "  }\n"
        "}\n"
    ));

    EXPECT_PARSE_ERROR(parseActorDef_(
        "function test2(){return 4;}\n"
    ));

    EXPECT_PARSE_ERROR(parseActorDef_(
        "actor 1a{\n"
        "  output o1(int)\n"
        "  input i1(a: int, b: int){\n"
        "    o1(a+b);\n"
        "  }\n"
        "}\n"
    ));

    EXPECT_PARSE_ERROR(parseActorDef_(
        "actor a1{\n"
        "  output 3o1(int)\n"
        "  input i1(a: int, b: int){\n"
        "    o1(a+b);\n"
        "  }\n"
        "}\n"
    ));

    auto r = parseActorDef_(
        "actor Test1 (a: int, b: bool){\n"
        "  output o1(int)\n"
        "  input i1(a: int, b: int){\n"
        "    o1(a+b);\n"
        "  }\n"
        "}\n"
    );
    ASSERT_PARSE_OK(r);
    EXPECT_EQ(AST_ACTOR, r.result->getType());
    ASSERT_EQ(3, r.result->childCount());
    EXPECT_EQ(AST_TUPLE_DEF, r.result->child(0)->getType());
    EXPECT_EQ(2, r.result->child(0)->childCount());
    EXPECT_EQ(AST_OUTPUT, r.result->child(1)->getType());
    EXPECT_EQ(AST_INPUT, r.result->child(2)->getType());

    r = parseActorDef_(
        "actor Test1 {\n"
        "  output o1(int)\n"
        "  input i1(a: int, b: int){\n"
        "    o1(a+b);\n"
        "  }\n"
        "}\n"
    );
    ASSERT_PARSE_OK(r);
    EXPECT_EQ(AST_ACTOR, r.result->getType());
    ASSERT_EQ(3, r.result->childCount());
    EXPECT_EQ(AST_TUPLE_DEF, r.result->child(0)->getType());
    EXPECT_EQ(0, r.result->child(0)->childCount());
    EXPECT_EQ(AST_OUTPUT, r.result->child(1)->getType());
    EXPECT_EQ(AST_INPUT, r.result->child(2)->getType());
}

/// <summary>
/// Tests for 'parseMsgHeader' function.
/// </summary>
TEST(Parser, parseMsgHeader)
{
    auto parseMsgHeader_ = [](const char* code)
    {
        return checkAllParsed(code, parseMsgHeader);
    };

    EXPECT_PARSE_OK(parseMsgHeader_("name1(a: int, b: int)"));
    EXPECT_PARSE_OK(parseMsgHeader_("name2()"));

    EXPECT_PARSE_ERROR(parseMsgHeader_("name();"));
    EXPECT_PARSE_ERROR(parseMsgHeader_("2name(x: int);"));
    EXPECT_PARSE_ERROR(parseMsgHeader_("name(1,2,3);"));

    auto r = parseMsgHeader_("name1(a: int, b: int)");
    ASSERT_PARSE_OK(r);
    auto node = r.result;

    EXPECT_STREQ("name1", node->getName().c_str());
    ASSERT_EQ(1, node->childCount());

    auto params = node->child(0);
    ASSERT_EQ(2, params->childCount());
    EXPECT_TRUE(params->child(0)->hasFlag(ASTF_FUNCTION_PARAMETER));
    EXPECT_TRUE(params->child(1)->hasFlag(ASTF_FUNCTION_PARAMETER));
    EXPECT_STREQ("a", params->child(0)->getName().c_str());
    EXPECT_STREQ("b", params->child(1)->getName().c_str());
}

/// <summary>
/// Tests for 'parseUnnamedInput' function.
/// </summary>
TEST(Parser, parseUnnamedInput)
{
    auto parseUnnamedInput_ = [](const char* code)
    {
        return checkAllParsed(code, parseUnnamedInput);
    };

    auto r = parseUnnamedInput_("actorA.out1 -> (a: int){}");
    ASSERT_PARSE_OK(r);
    auto node = r.result;

    ASSERT_EQ(3, node->childCount());
    EXPECT_EQ(AST_UNNAMED_INPUT, node->getType());
    EXPECT_EQ(AST_LIST, node->child(0)->getType());
    EXPECT_EQ(AST_TUPLE_DEF, node->child(1)->getType());
    EXPECT_EQ(AST_BLOCK, node->child(2)->getType());

    EXPECT_PARSE_OK(parseUnnamedInput_("actorA.o3 -> (){}"));

    //The following two test are errors suppossed to be handled by the semantic analyzer.
    EXPECT_PARSE_OK(parseUnnamedInput_("actorA -> (a: int){}"));
    EXPECT_PARSE_OK(parseUnnamedInput_("-> (a: int){}"));

    EXPECT_PARSE_ERROR(parseUnnamedInput_("actorA.o1 -> {}"));
    EXPECT_PARSE_ERROR(parseUnnamedInput_("actorA,o1 -> (){}"));
    EXPECT_PARSE_ERROR(parseUnnamedInput_("actorA.o1 -> ()"));
    EXPECT_PARSE_ERROR(parseUnnamedInput_("actorA.o1 <- (){}"));
    EXPECT_PARSE_ERROR(parseUnnamedInput_("actorA.o1 (){}"));
}


/// <summary>
/// Tests for 'parseImport' function, which parses 'import' statements.
/// </summary>
TEST(Parser, parseImport)
{
    auto parseImport_ = [](const char* code)
    {
        return checkAllParsed(code, parseImport);
    };

    EXPECT_PARSE_OK(parseImport_("import \"ModuleA\""));
    EXPECT_PARSE_OK(parseImport_("import[C] \"cLibraryA\""));
    EXPECT_PARSE_ERROR(parseImport_("import 14"));
    EXPECT_PARSE_ERROR(parseImport_("import"));
    EXPECT_PARSE_ERROR(parseImport_("function \"ModuleA\""));
    EXPECT_PARSE_ERROR(parseImport_("\"ModuleA\""));

    auto r = parseImport_("import[C] \"test\"");
    auto node = r.result;

    EXPECT_EQ(0, node->childCount());
    EXPECT_STREQ("", node->getName().c_str());
    EXPECT_STREQ("test", node->getValue().c_str());
    EXPECT_TRUE(node->hasFlag(ASTF_EXTERN_C));
}
