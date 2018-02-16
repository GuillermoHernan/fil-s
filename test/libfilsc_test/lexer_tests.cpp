/// <summary>
/// Tests 'LexToken' class and its auxiliary functions.
/// </summary>

#include "libfilsc_test_pch.h"
#include "lexer.h"
#include "compileError.h"

/// <summary>
/// Tests 'tokenType2String' function.
/// </summary>
TEST(LexToken, tokenType2String)
{
	EXPECT_STREQ("EOF",		tokenType2String(LEX_EOF).c_str());
	EXPECT_STREQ("INITIAL",	tokenType2String(LEX_INITIAL).c_str());
	EXPECT_STREQ("COMMENT", tokenType2String(LEX_COMMENT).c_str());
	EXPECT_STREQ("NEWLINE", tokenType2String(LEX_NEWLINE).c_str());
	EXPECT_STREQ("ID",		tokenType2String(LEX_ID).c_str());
	EXPECT_STREQ("RESERVED", tokenType2String(LEX_RESERVED).c_str());
	EXPECT_STREQ("INT",		tokenType2String(LEX_INT).c_str());
	EXPECT_STREQ("FLOAT",	tokenType2String(LEX_FLOAT).c_str());
	EXPECT_STREQ("STRING",	tokenType2String(LEX_STR).c_str());
	EXPECT_STREQ("OPERATOR", tokenType2String(LEX_OPERATOR).c_str());

	EXPECT_STREQ("?[1234]", tokenType2String((LEX_TYPES)1234).c_str());
}

/// <summary>
/// Tests the constructor which initializes the token from a code string.
/// </summary>
TEST(LexToken, ConstructorFromCode)
{
	const char* code = "some code";

	LexToken	tok(code);

	EXPECT_EQ(LEX_INITIAL, tok.type());
	EXPECT_EQ(1, tok.getPosition().column);
	EXPECT_EQ(1, tok.getPosition().line);
	EXPECT_STREQ("", tok.text().c_str());
}

/// <summary>
/// Tests 'strValue' function, which decodes a string in source format.
/// </summary>
TEST(LexToken, strValue)
{
	const char *inputStr = "\"\\tTest string\\r\\n\\b\\f\\\\\\v\\n"
		"\\m\\x4A\\113\\'\"";
	const char* expected = "\tTest string\r\n\b\f\\\v\n"
		"mJK'";

	LexToken tok(inputStr);
	tok = tok.next();

	ASSERT_EQ(LEX_STR, tok.type());
	EXPECT_STREQ(expected, tok.strValue().c_str());

	//Invalid hex escape sequence
	tok = LexToken("\"A\\xZ\"");
	tok = tok.next();

	ASSERT_EQ(LEX_STR, tok.type());
	EXPECT_THROW(tok.strValue(), CompileError);
}

/// <summary>
/// Tests 'next' function, which returns the next token in the stream
/// </summary>
TEST(LexToken, next)
{
	const char* code = "//This is a comment\n"
		"x = 10;";

	LexToken tok(code);
	LexToken next = tok.next(LexToken::NONE);

	EXPECT_EQ(LEX_ID, next.type());
	EXPECT_STREQ("x", next.text().c_str());

	next = tok.next(LexToken::COMMENTS);
	EXPECT_EQ(LEX_COMMENT, next.type());
	EXPECT_STREQ("//This is a comment", next.text().c_str());

	next = tok.next(LexToken::NEWLINE);
	EXPECT_EQ(LEX_NEWLINE, next.type());
	EXPECT_STREQ("\n", next.text().c_str());
}

/// <summary>
/// Tests 'nextDispatch' function. This is an internal function which decides which
/// kind of token is the next one based on the first character.
/// </summary>
TEST(LexToken, nextDispatch)
{
	const char *code = "//This is a comment\n"
		"x = 12 / 3\n"
		"y = \"test\"";

	LexToken	tok(code);

	//'match' would throw and exception if the appropriate token type is not found.
	tok = tok.match(LEX_INITIAL, LexToken::ALL);
	tok = tok.match(LEX_COMMENT, LexToken::ALL);
	tok = tok.match(LEX_NEWLINE, LexToken::ALL);
	tok = tok.match(LEX_ID, LexToken::ALL);
	tok = tok.match(LEX_OPERATOR, LexToken::ALL);
	tok = tok.match(LEX_INT, LexToken::ALL);
	tok = tok.match(LEX_OPERATOR, LexToken::ALL);
	tok = tok.match(LEX_INT, LexToken::ALL);
	tok = tok.match(LEX_NEWLINE, LexToken::ALL);
	tok = tok.match(LEX_ID, LexToken::ALL);
	tok = tok.match(LEX_OPERATOR, LexToken::ALL);
	tok = tok.match(LEX_STR, LexToken::ALL);

	EXPECT_EQ(LEX_EOF, tok.type());
}

/// <summary>
/// Tests 'match' function. This function throws an exception if the expected token
/// is not the current one
/// </summary>
TEST(LexToken, match)
{
	const char *code = "z = y + x";

	LexToken	tok(code);

	tok = tok.next();
	tok = tok.match(LEX_ID);
	tok = tok.match(LEX_OPERATOR);

	EXPECT_THROW(tok.match(LEX_INT), CompileError);

	tok = tok.match(LEX_ID, "y");
	EXPECT_THROW(tok.match(LEX_INT, "12"), CompileError);
	EXPECT_THROW(tok.match(LEX_OPERATOR, "-"), CompileError);
}

/// <summary>
/// Tests 'parseComment' function. This is the internal function which matches comments.
/// </summary>
TEST(LexToken, parseComment)
{
	const char *code = "//Comment A\n"
		"/*\n"
		"\tComment B*/\n"
		"//Comment C";

	LexToken	tok(code);

	tok = tok.next(LexToken::COMMENTS);
	EXPECT_EQ(LEX_COMMENT, tok.type());
	EXPECT_STREQ("//Comment A", tok.text().c_str());

	tok = tok.next(LexToken::COMMENTS);
	EXPECT_EQ(LEX_COMMENT, tok.type());
	EXPECT_STREQ("/*\n\tComment B*/", tok.text().c_str());

	tok = tok.next(LexToken::COMMENTS);
	EXPECT_EQ(LEX_COMMENT, tok.type());
	EXPECT_STREQ("//Comment C", tok.text().c_str());

	tok = tok.next(LexToken::COMMENTS);
	EXPECT_EQ(LEX_EOF, tok.type());

	// Unclosed multi line comment
	tok = LexToken("/*Unclosed");

	EXPECT_THROW(tok.next(), CompileError);
}

/// <summary>
/// Tests 'parseID' function. This is the internal function which matches 
/// identifiers and keywords.
/// </summary>
TEST(LexToken, parseID)
{
	const char *code = "a aa bZ2 z2w _a_e_ if a' a''";

	LexToken	tok(code);

	tok = tok.next();
	EXPECT_STREQ("a", tok.text().c_str());

	tok = tok.match(LEX_ID);
	EXPECT_STREQ("aa", tok.text().c_str());

	tok = tok.match(LEX_ID);
	EXPECT_STREQ("bZ2", tok.text().c_str());

	tok = tok.match(LEX_ID);
	EXPECT_STREQ("z2w", tok.text().c_str());

	tok = tok.match(LEX_ID);
	EXPECT_STREQ("_a_e_", tok.text().c_str());

	tok = tok.match(LEX_ID);
	EXPECT_STREQ("if", tok.text().c_str());
	
	tok = tok.match(LEX_RESERVED);
	EXPECT_STREQ("a'", tok.text().c_str());

	tok = tok.match(LEX_ID);
	EXPECT_STREQ("a''", tok.text().c_str());
}


/// <summary>
/// Tests 'parseNumber' function. This is the internal function which matches 
/// both integer and floating point literals.
/// </summary>
TEST(LexToken, parseNumber)
{
	const char *code = "0xF07D\n"
		"1356\n"
		"13.56\n"
		"14.7e3\n"
		"14.7e+3\n"
		"14.7e-3\n"
		"2e+3\n"
		"2e-3\n";

	LexToken	tok(code);

	tok = tok.next();
	EXPECT_EQ(LEX_INT, tok.type());
	EXPECT_STREQ("0xF07D", tok.text().c_str());

	tok = tok.next();
	EXPECT_EQ(LEX_INT, tok.type());
	EXPECT_STREQ("1356", tok.text().c_str());

	tok = tok.next();
	EXPECT_EQ(LEX_FLOAT, tok.type());
	EXPECT_STREQ("13.56", tok.text().c_str());

	tok = tok.next();
	EXPECT_EQ(LEX_FLOAT, tok.type());
	EXPECT_STREQ("14.7e3", tok.text().c_str());

	tok = tok.next();
	EXPECT_EQ(LEX_FLOAT, tok.type());
	EXPECT_STREQ("14.7e+3", tok.text().c_str());

	tok = tok.next();
	EXPECT_EQ(LEX_FLOAT, tok.type());
	EXPECT_STREQ("14.7e-3", tok.text().c_str());

	tok = tok.next();
	EXPECT_EQ(LEX_FLOAT, tok.type());
	EXPECT_STREQ("2e+3", tok.text().c_str());

	tok = tok.next();
	EXPECT_EQ(LEX_FLOAT, tok.type());
	EXPECT_STREQ("2e-3", tok.text().c_str());

	tok = tok.next();
	EXPECT_EQ(LEX_EOF, tok.type());
}


/// <summary>
/// Tests 'parseString' function. This is the internal function which matches 
/// string literals.
/// </summary>
TEST(LexToken, parseString)
{
	const char *code = "\"test A\"\n"
		"\"\\\"test B\\\"\"\n";

	LexToken	tok(code);

	tok = tok.next();
	EXPECT_EQ(LEX_STR, tok.type());
	EXPECT_STREQ("test A", tok.strValue().c_str());

	tok = tok.next();
	EXPECT_EQ(LEX_STR, tok.type());
	EXPECT_STREQ("\"test B\"", tok.strValue().c_str());

	tok = tok.next();
	EXPECT_EQ(LEX_EOF, tok.type());

	//New line in string literal
	tok = LexToken("\"test\nC\"");
	EXPECT_THROW(tok.next(), CompileError);

	//End of file in string literal
	tok = LexToken("\"test D");
	EXPECT_THROW(tok.next(), CompileError);
}

/// <summary>
/// Tests 'parseOperator' function. This is the internal function which matches 
/// operators.
/// </summary>
TEST(LexToken, parseOperator)
{
	const char *code = ">>>= ===  !==  >>>  <<=  >>= **=  ==  != \n"
		"<=  >=  <<  >>  **  +=  -=  *=  /=  %=  &=  |=  ^=  || \n"
		"&&  ++  --  <-  -> \n"
		"! | % & / () ? = [] {} . , ; : < > + * ^ ~";

	LexToken	tok(code);

	tok = tok.next();
	tok = tok.match(LEX_OPERATOR, ">>>=");
	tok = tok.match(LEX_OPERATOR, "===");
	tok = tok.match(LEX_OPERATOR, "!==");
	tok = tok.match(LEX_OPERATOR, ">>>");
	tok = tok.match(LEX_OPERATOR, "<<=");
	tok = tok.match(LEX_OPERATOR, ">>=");
	tok = tok.match(LEX_OPERATOR, "**=");
	tok = tok.match(LEX_OPERATOR, "==");
	tok = tok.match(LEX_OPERATOR, "!=");
	tok = tok.match(LEX_OPERATOR, "<=");
	tok = tok.match(LEX_OPERATOR, ">=");
	tok = tok.match(LEX_OPERATOR, "<<");
	tok = tok.match(LEX_OPERATOR, ">>");
	tok = tok.match(LEX_OPERATOR, "**");
	tok = tok.match(LEX_OPERATOR, "+=");
	tok = tok.match(LEX_OPERATOR, "-=");
	tok = tok.match(LEX_OPERATOR, "*=");
	tok = tok.match(LEX_OPERATOR, "/=");
	tok = tok.match(LEX_OPERATOR, "%=");
	tok = tok.match(LEX_OPERATOR, "&=");
	tok = tok.match(LEX_OPERATOR, "|=");
	tok = tok.match(LEX_OPERATOR, "^=");
	tok = tok.match(LEX_OPERATOR, "||");
	tok = tok.match(LEX_OPERATOR, "&&");
	tok = tok.match(LEX_OPERATOR, "++");
	tok = tok.match(LEX_OPERATOR, "--");
	tok = tok.match(LEX_OPERATOR, "<-");
	tok = tok.match(LEX_OPERATOR, "->");
	tok = tok.match(LEX_OPERATOR, "!");
	tok = tok.match(LEX_OPERATOR, "|");
	tok = tok.match(LEX_OPERATOR, "%");
	tok = tok.match(LEX_OPERATOR, "&");
	tok = tok.match(LEX_OPERATOR, "/");
	tok = tok.match(LEX_OPERATOR, "(");
	tok = tok.match(LEX_OPERATOR, ")");
	tok = tok.match(LEX_OPERATOR, "?");
	tok = tok.match(LEX_OPERATOR, "=");
	tok = tok.match(LEX_OPERATOR, "[");
	tok = tok.match(LEX_OPERATOR, "]");
	tok = tok.match(LEX_OPERATOR, "{");
	tok = tok.match(LEX_OPERATOR, "}");
	tok = tok.match(LEX_OPERATOR, ".");
	tok = tok.match(LEX_OPERATOR, ",");
	tok = tok.match(LEX_OPERATOR, ";");
	tok = tok.match(LEX_OPERATOR, ":");
	tok = tok.match(LEX_OPERATOR, "<");
	tok = tok.match(LEX_OPERATOR, ">");
	tok = tok.match(LEX_OPERATOR, "+");
	tok = tok.match(LEX_OPERATOR, "*");
	tok = tok.match(LEX_OPERATOR, "^");
	tok = tok.match(LEX_OPERATOR, "~");

	EXPECT_TRUE(tok.eof());
}


/// <summary>
/// Tests 'calcPosition' function. Calculates the positions of the tokens
/// in terms of lines / columns
/// </summary>
TEST(LexToken, calcPosition)
{
	const char *code = "r = 2 + 3\n\n"
		"z=r*7\n";

	LexToken	tok(code);

	EXPECT_EQ(1, tok.getPosition().line);
	EXPECT_EQ(1, tok.getPosition().column);

	tok = tok.next();
	EXPECT_STREQ("r", tok.text().c_str());
	EXPECT_EQ(1, tok.getPosition().line);
	EXPECT_EQ(1, tok.getPosition().column);

	tok = tok.next();
	EXPECT_STREQ("=", tok.text().c_str());
	EXPECT_EQ(1, tok.getPosition().line);
	EXPECT_EQ(3, tok.getPosition().column);

	tok = tok.next();
	EXPECT_STREQ("2", tok.text().c_str());
	EXPECT_EQ(1, tok.getPosition().line);
	EXPECT_EQ(5, tok.getPosition().column);

	tok = tok.next();
	EXPECT_STREQ("+", tok.text().c_str());
	EXPECT_EQ(1, tok.getPosition().line);
	EXPECT_EQ(7, tok.getPosition().column);

	tok = tok.next();
	EXPECT_STREQ("3", tok.text().c_str());
	EXPECT_EQ(1, tok.getPosition().line);
	EXPECT_EQ(9, tok.getPosition().column);

	tok = tok.next();
	EXPECT_STREQ("z", tok.text().c_str());
	EXPECT_EQ(3, tok.getPosition().line);
	EXPECT_EQ(1, tok.getPosition().column);

	tok = tok.next();
	EXPECT_STREQ("=", tok.text().c_str());
	EXPECT_EQ(3, tok.getPosition().line);
	EXPECT_EQ(2, tok.getPosition().column);

	tok = tok.next();
	EXPECT_STREQ("r", tok.text().c_str());
	EXPECT_EQ(3, tok.getPosition().line);
	EXPECT_EQ(3, tok.getPosition().column);

	tok = tok.next();
	EXPECT_STREQ("*", tok.text().c_str());
	EXPECT_EQ(3, tok.getPosition().line);
	EXPECT_EQ(4, tok.getPosition().column);

	tok = tok.next();
	EXPECT_STREQ("7", tok.text().c_str());
	EXPECT_EQ(3, tok.getPosition().line);
	EXPECT_EQ(5, tok.getPosition().column);

	tok = tok.next();
	EXPECT_TRUE(tok.eof());
}

/// <summary>
/// Test 'isOperator' function, with checks if the current token is an operator.
/// </summary>
/// <param name=""></param>
/// <param name=""></param>
/// <returns></returns>
TEST(LexToken, isOperator)
{
	const char *code = "= a";

	LexToken	tok(code);

	tok = tok.next();
	EXPECT_TRUE(tok.isOperator("="));
	EXPECT_FALSE(tok.isOperator("+"));
	EXPECT_FALSE(tok.isOperator("=="));

	tok = tok.next();
	EXPECT_FALSE(tok.isOperator("="));
	EXPECT_FALSE(tok.isOperator("a"));

	tok = tok.next();
	EXPECT_TRUE(tok.eof());
}
