/// <summary>
/// Tests for the type check compiler pass.
/// </summary>

#include "libfilsc_test_pch.h"
#include "typeCheckPass.h"
#include "semAnalysisState.h"
#include "semanticAnalysis.h"

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

    auto node = findNode(r.result, [](auto node) {
        return node->getType() == AST_TUPLE_DEF;
    });

    ASSERT_TRUE(node.notNull());

    auto dataType = node->getDataType();
    EXPECT_TRUE(astIsTupleType(dataType));
    EXPECT_DATATYPE_STR("(int,bool,(int,int))", dataType);
}

/// <summary>Tests 'blockTypeCheck' function.</summary>
TEST(TypeCheck, blockTypeCheck)
{
    auto r = semAnalysisCheck("const a = {}");

    ASSERT_SEM_OK(r);

    auto nodes = findNodes(r.result, [](auto node) {
        return node->getType() == AST_BLOCK || node->getType() == AST_DECLARATION;
    });

    ASSERT_EQ(2, nodes.size());
    EXPECT_TRUE(astIsVoidType(nodes[0]->getDataType()));
    EXPECT_TRUE(astIsVoidType(nodes[1]->getDataType()));

    r = semAnalysisCheck("const a = {const b = (1*4)+9; (b,false)}");

    ASSERT_SEM_OK(r);

    auto node = findNode(r.result, [](auto node) {
        return node->getType() == AST_BLOCK;
    });

    ASSERT_TRUE(node.notNull());
    EXPECT_TRUE(astIsTupleType(node->getDataType()));
    EXPECT_DATATYPE_STR("(int,bool)", node->getDataType());

    //printAST(r.result, cout);
}

/// <summary>Tests 'tupleTypeCheck' function.</summary>
TEST(TypeCheck, tupleTypeCheck)
{
    auto r = semAnalysisCheck("const a = (1,2,true)");

    ASSERT_SEM_OK(r);

    auto node = findNode(r.result, [](auto node) {
        return node->getType() == AST_TUPLE;
    });

    ASSERT_TRUE(node.notNull());
    EXPECT_TRUE(astIsTupleType(node->getDataType()));

    EXPECT_DATATYPE_STR("(int,int,bool)", node->getDataType());

    //printAST(r.result, cout);
}

/// <summary>Tests 'declarationTypeCheck' function.
/// Also tests 'recursiveSymbolReferenceCheck' function.</summary>
TEST(TypeCheck, declarationTypeCheck)
{
    auto findDeclaration = [](Ref<AstNode> root) {
        return findNode(root, AST_DECLARATION);
    };

    auto r = semAnalysisCheck("const a:int;");

    ASSERT_SEM_OK(r);
    auto decl = findDeclaration(r.result);
    EXPECT_DATATYPE_STR("int", decl->getDataType());

    EXPECT_SEM_OK(semAnalysisCheck("const a:int = 5;"));
    EXPECT_SEM_ERROR(semAnalysisCheck("const a:int = true;"));

    //Inferred type
    r = semAnalysisCheck("const a = (true, 4);");

    ASSERT_SEM_OK(r);
    decl = findDeclaration(r.result);
    EXPECT_DATATYPE_STR("(bool,int)", decl->getDataType());

    r = semAnalysisCheck("const a;");
    ASSERT_SEM_ERROR(r);
    EXPECT_EQ(ETYPE_DECLARATION_WITHOUT_TYPE, r.errors[0].type());

    r = semAnalysisCheck("const a = a + 1");

    ASSERT_SEM_ERROR(r);
    EXPECT_EQ(ETYPE_RECURSIVE_SYMBOL_REFERENCE_1, r.errors[0].type());
}


/// <summary>Tests 'ifTypeCheck' function.</summary>
TEST(TypeCheck, ifTypeCheck)
{
    auto r = semAnalysisCheck("const a = if (1>7) (4,5) else (6,7);");

    ASSERT_SEM_OK(r);
    auto node = findNode(r.result, AST_IF);
    EXPECT_DATATYPE_STR("(int,int)", node->getDataType());

    r = semAnalysisCheck("const a = if (false) (4,5) else (false,7);");

    ASSERT_SEM_OK(r);
    node = findNode(r.result, AST_IF);
    EXPECT_DATATYPE_STR("()", node->getDataType());

    r = semAnalysisCheck("const a = if (true) 4;");

    ASSERT_SEM_OK(r);
    node = findNode(r.result, AST_IF);
    EXPECT_DATATYPE_STR("()", node->getDataType());

    r = semAnalysisCheck("const a = if (0) 3 else 5;");

    ASSERT_SEM_ERROR(r);
    EXPECT_EQ(ETYPE_WRONG_IF_CONDITION_TYPE_1, r.errors[0].type());
}

/// <summary>Tests 'returnTypeAssign' function.</summary>
TEST(TypeCheck, returnTypeAssign)
{
    auto r = semAnalysisCheck("function f() {return (9,4);}");

    ASSERT_SEM_OK(r);
    auto node = findNode(r.result, AST_RETURN);
    EXPECT_DATATYPE_STR("(int,int)", node->getDataType());

    r = semAnalysisCheck("function f() {return;}");

    ASSERT_SEM_OK(r);
    node = findNode(r.result, AST_RETURN);
    EXPECT_DATATYPE_STR("()", node->getDataType());
}

/// <summary>Tests 'functionDefTypeCheck' function.</summary>
TEST(TypeCheck, functionDefTypeCheck)
{
    //Function with declared type and matching body type.
    auto r = semAnalysisCheck("function div(a: int, b: int):(int, int){\n"
        "const x = a/b\n"
        "const r = a - (x*b)\n"
        "(x,r)}");

    //printAST(r.result, cout, 0);
    ASSERT_SEM_OK(r);
    auto node = findNode(r.result, AST_FUNCTION);
    EXPECT_DATATYPE_STR("function(int,int):(int,int)", node->getDataType());

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
    node = findNode(r.result, AST_FUNCTION);
    EXPECT_DATATYPE_STR("function(int,int):(int,int,bool)", node->getDataType());
}

/// <summary>Tests 'assignmentTypeCheck' function.</summary>
TEST(TypeCheck, assignmentTypeCheck)
{
    auto r = semAnalysisCheck("function f() {var a:int; a = 7;}");
    ASSERT_SEM_OK(r);

    auto node = findNode(r.result, AST_ASSIGNMENT);
    ASSERT_TRUE(node.notNull());
    EXPECT_DATATYPE_STR("int", node->getDataType());

    ASSERT_SEM_ERROR(semAnalysisCheck("function f() {var a:bool; a = 7;}"));
}

/// <summary>Tests 'callTypeCheck' function.</summary>
TEST(TypeCheck, callTypeCheck)
{
    auto r = semAnalysisCheck("function add(a:int, b:int):int {a+b}\n"
        "function test():() {const x = add(5,7);}");
    ASSERT_SEM_OK(r);

    //printAST(r.result, cout);

    auto node = findNode(r.result, AST_FNCALL);
    ASSERT_TRUE(node.notNull());
    EXPECT_DATATYPE_STR("int", node->getDataType());
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
    //printAST(r.result, cout);
    ASSERT_SEM_OK(r);

    auto node = findNode(r.result, AST_MEMBER_ACCESS);
    ASSERT_TRUE(node.notNull());
    EXPECT_DATATYPE_STR("int", node->getDataType());
    EXPECT_DATATYPE_STR("(int,int)", node->child(0)->getDataType());

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


/// <summary>Tests binary operations type checking</summary>
TEST(TypeCheck, binaryOpTypeCheck)
{
    auto check = [](const string& codeFragment) {
        return semAnalysisCheck(("function test():() {\n" + codeFragment + "\n};\n").c_str());
    };

    EXPECT_SEM_OK(check("3+3; 3-1; 4*5; 12/4; 17 % 5;"));
    EXPECT_SEM_ERROR(check("3+true"));
    EXPECT_SEM_ERROR(check("false/true"));
    EXPECT_SEM_ERROR(check("false-4"));

    EXPECT_SEM_OK(check(
        "var a:int\n"
        "a = 6<<1; a = 32>>3; a = 0xf & 4; a = 1 | 2; a = 0xff ^ 0xaa;"
    ));

    EXPECT_SEM_ERROR(check("3|true"));
    EXPECT_SEM_ERROR(check("false&true"));
    EXPECT_SEM_ERROR(check("false >> 4"));

    EXPECT_SEM_OK(check(
        "var a:bool\n"
        "a = 6<1; a = 7>3; a = 5 == 5; a = 6 != 7; a = 5>=7; a = 6 <= 6;\n"
        "a = true == true; a = false != true;"
    ));

    EXPECT_SEM_ERROR(check("const a:int = 9 != 4"));
    EXPECT_SEM_ERROR(check("1 != true"));
    EXPECT_SEM_ERROR(check("false >= true"));
    EXPECT_SEM_ERROR(check("false == 0"));

    EXPECT_SEM_OK(check(
        "var a:bool\n"
        "a = (6>5) || (7<9); a = true && false"
    ));

    EXPECT_SEM_ERROR(check("false || 7"));
    EXPECT_SEM_ERROR(check("9 && 9"));
    EXPECT_SEM_ERROR(check("14 || true"));

    //TODO: Enable leak checking. I know that the AST leaks (a lot of) memory,
    //due to the circular references created by the scope links and type anotations.
    //But I do not see it as a major problem at this moment.
    //Check node leaks
    //EXPECT_EQ(0, AstNode::nodeCount());
}


/// <summary>Tests prefix operators type checking</summary>
TEST(TypeCheck, prefixOpTypeCheck)
{
    auto check = [](const string& codeFragment) {
        return semAnalysisCheck(("function test():() {\n" + codeFragment + "\n};\n").c_str());
    };

    EXPECT_SEM_OK(check(
        "  var b = true;\n"
        "  var x = 5;\n"
        "  var y: int\n"
        "  b = !b;\n"
        "  y = -x;\n"
        "  y = +y;\n"
        "  x = --y;\n"
        "  y = ++x;\n"
        "  --y;\n"
    ));

    EXPECT_SEM_ERROR(check("var x = !3;\n"));
    EXPECT_SEM_ERROR(check("var x = -true;\n"));
}


/// <summary>Tests 'returnTypeCheck' function.</summary>
TEST(TypeCheck, returnTypeCheck)
{
    EXPECT_SEM_OK(semAnalysisCheck("function f() {return (9,4);}"));

    auto r = semAnalysisCheck(
        "function f(a:int):int {\n"
        "  if (a > 7) return (9,4);\n"
        "  16\n"
        "}"
    );
    ASSERT_SEM_ERROR(r);
    EXPECT_EQ(ETYPE_INCOMPATIBLE_RETURN_TYPE_2, r.errors[0].type());

    r = semAnalysisCheck("const a = {return (9,4)}");
    ASSERT_SEM_ERROR(r);
    EXPECT_EQ(ETYPE_RETURN_OUTSIDE_FUNCTION, r.errors[0].type());
}


/// <summary>
/// Tests 'actorTypeCheck' function.
/// Also tests 'messageTypeCheck' function
/// </summary>
TEST(TypeCheck, actorTypeCheck)
{
    EXPECT_SEM_OK(semAnalysisCheck(
        "actor A1 {\n"
        "  output o1(v : int)\n"
        "  input i1(a : int){\n"
        "    o1(a*a);\n"
        "  }\n"
        "}"
    ));

    EXPECT_SEM_OK(semAnalysisCheck(
        "actor A1 {\n"
        "  output o1(int)\n"
        "  input i1(a : int){\n"
        "    o1(a*a);\n"
        "  }\n"
        "}"
    ));

    EXPECT_SEM_OK(semAnalysisCheck(
        "actor A2 (p1: int, p2: int) {\n"
        "  output o1(r : int)\n"
        "  input i1(x : int){\n"
        "    o1((x*p1) + p2);\n"
        "  }\n"
        "}"
    ));

    EXPECT_SEM_OK(semAnalysisCheck(
        "actor A2 (p1: int, p2: int) {\n"
        "  const c1 = p1 + (p2*2)\n"
        "  output o1(r : int)\n"
        "  input i1(x : int){\n"
        "    o1((x*p1) + c1);\n"
        "  }\n"
        "}"
    ));

    EXPECT_SEM_ERROR(semAnalysisCheck(
        "actor A2 (p1: int, p2: int) {\n"
        "  const p1 = p2*2\n"
        "  output o1(r : int)\n"
        "  input i1(x : int){\n"
        "    o1((x*p1) + p2);\n"
        "  }\n"
        "}"
    ));

    EXPECT_SEM_ERROR(semAnalysisCheck(
        "actor A1 {\n"
        "  output o1(v : int, b: bool)\n"
        "  input i1(a : int){\n"
        "    o1(a*a);\n"
        "  }\n"
        "}"
    ));

    EXPECT_SEM_ERROR(semAnalysisCheck(
        "actor A1 {\n"
        "  const integer = 45;\n"
        "  output o1(int)\n"
        "  output o2(integer)\n"
        "  input i1(a : int){\n"
        "    o1(a*a);\n"
        "  }\n"
        "}"
    ));

    EXPECT_SEM_ERROR(semAnalysisCheck(
        "actor A1 {\n"
        "  output o1(v : int)\n"
        "  input i1(a : int){\n"
        "    wrongOutput(a*a);\n"
        "  }\n"
        "}"
    ));
}


/// <summary>
/// Tests 'actorInstanceTypeCheck' function.
/// </summary>
TEST(TypeCheck, actorInstanceTypeCheck)
{
    EXPECT_SEM_OK(semAnalysisCheck(
        "actor A {\n"
        "  output o1(v : int)\n"
        "  input i1(a : int){\n"
        "    o1(a*a);\n"
        "  }\n"
        "}\n"
        "\n"
        "actor B {\n"
        "  const aInstance:A\n"
        "  output o1(v : int)\n"
        "  input i1(a : int){\n"
        "    o1(a*a);\n"
        "  }\n"
        "}\n"
    ));

    EXPECT_SEM_OK(semAnalysisCheck(
        "actor A(p1: int) {\n"
        "  output o1(v : int)\n"
        "  input i1(a : int){\n"
        "    o1((a*a) + p1);\n"
        "  }\n"
        "}\n"
        "\n"
        "actor B {\n"
        "  const aInstance = A(3)\n"
        "  output o1(v : int)\n"
        "  input i1(a : int){\n"
        "    o1(a*a);\n"
        "  }\n"
        "}\n"
    ));

    EXPECT_SEM_OK(semAnalysisCheck(
        "actor A(p1: int, p2:int) {\n"
        "  output o1(v : int)\n"
        "  input i1(a : int){\n"
        "    o1((a*p2) + p1);\n"
        "  }\n"
        "}\n"
        "\n"
        "actor B {\n"
        "  const aInstance = A (0,3)\n"
        "  output o1(v : int)\n"
        "  input i1(a : int){\n"
        "    o1(a*a);\n"
        "  }\n"
        "}\n"
    ));

    auto r = semAnalysisCheck(
        "actor A {\n"
        "  output o1(v : int)\n"
        "  input i1(a : int){\n"
        "    o1(a*a);\n"
        "  }\n"
        "}\n"
        "\n"
        "const aInstance:A\n"
        "\n"
    );
    ASSERT_SEM_ERROR(r);
    EXPECT_EQ(ETYPE_MISPLACED_ACTOR_INSTANCE, r.errors[0].type());

    //auto r = semAnalysisCheck(
    //	"actor A {\n"
    //	"  const aInstance:A\n"
    //	"  output o1(v : int)\n"
    //	"  input i1(a : int){\n"
    //	"    o1(a*a);\n"
    //	"  }\n"
    //	"}\n"
    //	"\n"
    //);
    //ASSERT_SEM_ERROR(r);
    //EXPECT_EQ(ETYPE_RECURSIVE_ACTOR_INSTANCE, r.errors[0].type());

    r = semAnalysisCheck(
        "actor A {\n"
        "  output o1(v : int)\n"
        "  input i1(a : int){\n"
        "    o1(a*a);\n"
        "  }\n"
        "}\n"
        "\n"
        "actor B {\n"
        "  var aInstance:A\n"
        "  output o1(v : int)\n"
        "  input i1(a : int){\n"
        "    o1(a*a);\n"
        "  }\n"
        "}\n"
    );
    ASSERT_SEM_ERROR(r);
    EXPECT_EQ(ETYPE_NON_CONST_ACTOR_INSTANCE, r.errors[0].type());
}


/// <summary>
/// Tests 'unnamedInputTypeCheck' function.
/// Also tests 'getConnectOutputType' function
/// </summary>
TEST(TypeCheck, unnamedInputTypeCheck)
{
    EXPECT_SEM_OK(semAnalysisCheck(
        "actor A {\n"
        "  output o1(int)\n"
        "  input i1(a : int){}\n"
        "}\n"
        "\n"
        "actor B {\n"
        "  const ai:A\n"
        "  ai.o1 -> (a: int){}"
        "}\n"
    ));

    auto r = semAnalysisCheck(
        "actor A {\n"
        "  ai.o1 -> (a: int){}"
        "}\n"
    );
    ASSERT_SEM_ERROR(r);
    EXPECT_EQ(ETYPE_INVALID_CONNECT_OUTPUT, r.errors[0].type());

    r = semAnalysisCheck(
        "actor A {\n"
        "  -> (a: int){}"
        "}\n"
    );
    ASSERT_SEM_ERROR(r);
    EXPECT_EQ(ETYPE_UNSPECIFIED_CONNECT_OUTPUT, r.errors[0].type());

    r = semAnalysisCheck(
        "actor A {\n"
        "  output o1(int)\n"
        "  input i1(a : int){}\n"
        "}\n"
        "\n"
        "actor B {\n"
        "  const ai:A\n"
        "  ai.o2 -> (a: int){}\n"
        "  ai.i1 -> (a: int){}\n"
        "}\n"
    );
    ASSERT_SEM_ERROR(r);
    ASSERT_LE((size_t)2, r.errors.size());
    EXPECT_EQ(ETYPE_INVALID_CONNECT_OUTPUT, r.errors[0].type());
    EXPECT_EQ(ETYPE_INVALID_CONNECT_OUTPUT, r.errors[1].type());
}