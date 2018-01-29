/// <summary>
/// Lexical analyzer for FIL-S language
/// </summary>

#include "pch.h"
#include "lexer.h"
#include "utils.h"

#include "compileError.h"

using namespace std;

string tokenType2String(int token)
{
    if (token > 32 && token < 128)
    {
        char buf[2] = "x";
        buf[0] = (char)token;
        return buf;
    }

	switch (token)
    {
    case LEX_EOF: return "EOF";
	case LEX_INITIAL: return "INITIAL";
	case LEX_COMMENT: return "COMMENT";
	case LEX_ID: return "ID";
	case LEX_INT: return "INT";
    case LEX_FLOAT: return "FLOAT";
	case LEX_STR: return "STRING";
	case LEX_OPERATOR: return "OPERATOR";
    
    }

    ostringstream msg;
    msg << "?[" << token << "]";
    return msg.str();
}

/// <summary>Constructor which receives a source code string.</summary>
/// <remarks>
/// The constructor doesn't make a copy of the input string, so it is important
/// not to delete input string while there are still live 'LexToken's using it.
/// 
/// The token created with the constructor is not parsed from input string. It is
/// just the 'initial' token.To parse the first real token, call 'next'.
/// </remarks>
LexToken::LexToken(const char* code)
: m_code(code)
, m_type(LEX_INITIAL)
, m_position(1, 1)
, m_length(0)
{
}

LexToken::LexToken(LEX_TYPES lexType, const char* code, const ScriptPosition& position, int length)
: m_code(code)
, m_type(lexType)
, m_position(position)
, m_length(length)
{
}

/**
 * Returns full token text
 * @return 
 */
std::string LexToken::text()const
{
    return string(m_code, m_code + m_length);
}

/**
 * Gets the value of a string constant. Replaces escape sequences, and removes
 * initial and final quotes.
 * @return 
 */
std::string LexToken::strValue()const
{
    assert(type() == LEX_STR);

    string result;
    int i;

    result.reserve(m_length);

    for (i = 1; i < m_length - 1; ++i)
    {
        const char c = m_code[i];
        char buf[8];

        if (c != '\\')
            result.push_back(c);
        else
        {
            //TODO: Support for Unicode escape sequences.
            ++i;
            switch (m_code[i])
            {
            case 'b': result.push_back('\b');
                break;
            case 'f': result.push_back('\f');
                break;
            case 'n': result.push_back('\n');
                break;
            case 'r': result.push_back('\r');
                break;
            case 't': result.push_back('\t');
                break;
            case 'v': result.push_back('\v');
                break;
            case '\'': result.push_back('\'');
                break;
            case '\"': result.push_back('\"');
                break;
            case 'x':
                copyWhile(buf, m_code + i + 1, isHexadecimal, 2);
				if (buf[0] == 0)
					errorAt(m_code + i, "'\\x' escape sequence shall be followed by at least one heaxadecimal digit");
                result.push_back((char) strtol(buf, 0, 16));
				i += strlen(buf);
                break;
            default:
				copyWhile(buf, m_code + i, isOctal, 3);
				if (buf[0] != 0)
				{
					result.push_back((char)strtol(buf, 0, 8));
					i += strlen(buf);
				}
                else
                    result.push_back(m_code[i]);
            }//switch
        }
    }//for 

    return result;
}

/// <summary>
/// Checks if the current token is a particular operator.
/// </summary>
/// <param name="opText"></param>
/// <returns></returns>
bool LexToken::isOperator(const char* opText)const
{
	return m_type == LEX_OPERATOR && text() == opText;
}


/**
 * Creates a token, next to the current one, with the specified type
 * @param lexType
 * @param code
 * @param length
 * @return 
 */
LexToken LexToken::buildNextToken(LEX_TYPES lexType, const char* code, int length)const
{
    return LexToken(lexType, code, calcPosition(code), length);
}

/**
 * Reads next token from input, and returns it.
 * @param skipComments  To indicate if 'comment' tokens must be skipped
 * @return Next token
 */
LexToken LexToken::next(bool skipComments)const
{
    LexToken result = nextDispatch();

    if (skipComments && result.type() == LEX_COMMENT)
        return result.next(true);
    else
        return result;
}

/// Reads next token from input, and returns it.

LexToken LexToken::nextDispatch()const
{
    const char* currCh = skipWhitespace(m_code + m_length);

    if (*currCh == '/')
    {
        LexToken comment = parseComment(currCh);
        if (comment.type() != LEX_COMMENT)
            return parseOperator(currCh);
        else
            return comment;
    }
    else if (isAlpha(*currCh))
        return parseId(currCh);
    else if (isNumeric(*currCh))
        return parseNumber(currCh);
    else if (*currCh == '\"')
        return parseString(currCh);
    else if (*currCh != 0)
        return parseOperator(currCh);
    else
        return buildNextToken(LEX_EOF, currCh, 0);
}

LexToken LexToken::match(int expected_tk)const
{
    if (type() != expected_tk)
    {
        ostringstream errorString;
        errorString << "Got '" << text() << "' expected " << tokenType2String(expected_tk);
        return errorAt(m_code, "%s", errorString.str().c_str());
	}
	else
		return next();
}

/// <summary>
/// Checks that the current token matches the expected type and text.
/// </summary>
/// <param name="expected_tk">Expected token type</param>
/// <param name="expected_text">EXpected token text</param>
/// <returns>Next token</returns>
LexToken LexToken::match(int expected_tk, const char* expected_text)const
{
	if (type() != expected_tk || text() != expected_text)
	{
		ostringstream errorString;
		errorString << "Got '" << text() << "' expected '" << expected_text
			<< "'(" << tokenType2String(expected_tk) << ")";

		return errorAt(m_code, "%s", errorString.str().c_str());
	}
	else
		return next();

}


/**
* Parses commentaries. Both single line and multi-line
* @param code Pointer to comment code start.
* @return
*/
LexToken LexToken::parseComment(const char * const code)const
{
	const char* end = code + 2;

	if (code[1] == '/')
	{
		while (*end && *end != '\n')
			++end;
	}
	else if (code[1] == '*')
	{
		while (*end && (end[0] != '*' || end[1] != '/'))
			++end;

		if (*end == 0)
			errorAt(code, "Unclosed multi-line comment");
		else
			end += 2;
	}
	else
		return LexToken(""); //Not a commentary

	return buildNextToken(LEX_COMMENT, code, end - code);
}

/**
 * Parses an identifier
 * @param code Pointer to identifier start.
 * @return An identifier token
 */
LexToken LexToken::parseId(const char * code)const
{
    const char* end = code + 1;
    while (isAlpha(*end) || isNumeric(*end))
        ++end;

	//Identifiers can end with several 'single quote' characters.
	while (*end == '\'')
		++end;

    return buildNextToken(LEX_ID, code, end - code);
}

/**
 * Parses a number
 * @param code Pointer to number text start.
 * @return An integer or float token.
 */
LexToken LexToken::parseNumber(const char * code)const
{
    const char * end = code;
    LEX_TYPES type = LEX_INT;

    if (code[0] == '0' && tolower(code[1]) == 'x')
    {
        //Hexadecimal
        end = skipHexadecimal(code + 2);
    }
    else
    {
        //Decimal integers or floats.
        end = skipNumeric(code);

        if (*end == '.')
        {
            type = LEX_FLOAT;

            end = skipNumeric(end + 1);
        }

        // do fancy e-style floating point
        if (tolower(*end) == 'e')
        {
            type = LEX_FLOAT;

            if (end[1] == '+' || end[1] == '-')
                ++end;

            end = skipNumeric(end + 1);
        }
    }

    return buildNextToken(type, code, end - code);
}

/**
 * Parses a string constant
 * @param code Pointer to string constant start
 * @return A string token
 */
LexToken LexToken::parseString(const char * code)const
{
    const char openChar = *code;
    const char* end;

    for (end = code + 1; *end != openChar; ++end)
    {
        const char c = end[0];
        if (c == '\\' && end[1] != 0)
            ++end;
        else if (c == '\n' || c == '\r')
            errorAt(end, "New line in string constant");
        else if (c == 0)
            errorAt(end, "End of file in string constant");
    }

    return buildNextToken(LEX_STR, code, (end - code) + 1);
}

/**
 * Structure to define available operators
 */
struct SOperatorDef
{
    const char * text;
    const int len;
};

/**
 * Operator definition table. It is important that longer operators are added
 * before sorter ones.
 */
const SOperatorDef s_operators [] = {
    {">>>=", 4},
    {"===", 3},
    {"!==", 3},
    {">>>", 3},
    {"<<=", 3},
    {">>=", 3},
    {"**=", 3},
    {"==", 2},
    {"!=", 2},
    {"<=", 2},
    {">=", 2},
    {"<<", 2},
    {">>", 2},
    {"**", 2},
    {"+=", 2},
    {"-=", 2},
    {"*=", 2},
    {"/=", 2},
    {"%=", 2},
    {"&=", 2},
    {"|=", 2},
    {"^=", 2},
    {"||", 2},
    {"&&", 2},
    {"++", 2},
    {"--", 2},
    {"<-", 2},
    {"->", 2},
    {"", 0}, //Final record must have zero length
};

/**
 * Matches an operator token
 * @param code Pointer to the operator text
 * @return The appropriate token type for the operator
 */
LexToken LexToken::parseOperator(const char * code)const
{
    //First, try multi-char operators
    for (int i = 0; s_operators[i].len > 0; ++i)
    {
        if (strncmp(code, s_operators[i].text, s_operators[i].len) == 0)
            return buildNextToken(LEX_OPERATOR, code, s_operators[i].len);
    }

    //Take it as a single char operator
    return buildNextToken(LEX_OPERATOR, code, 1);
}

/**
 * Generates an error message located at the given position
 * @param code      Pointer to the code location where the error occurs. 
 * It is used to calculate line and column for the error message
 * @param msgFormat 'printf-like' format string
 * @param ...       Optional message parameters
 */
LexToken LexToken::errorAt(const char* code, const char* msgFormat, ...)const
{
    va_list aptr;

    va_start(aptr, msgFormat);
    ::errorAt_v(calcPosition(code), msgFormat, aptr);
    va_end(aptr);

    return *this;
}

/**
 * Calculates a line and column position, from a code pointer.
 * Uses the 'LexToken' information as a base from which to perform the 
 * calculations
 * @param codePos  Pointer to the text for which the position is going to be 
 * calculated. It MUST BE a pointer to the same text as 'm_code' pointer, and be
 * greater than it.
 */
ScriptPosition LexToken::calcPosition(const char* codePos)const
{
    assert(codePos >= m_code);

    int line = m_position.line, col = m_position.column;
    const char* code;

    for (code = m_code; *code != 0 && code < codePos; ++code, ++col)
    {
        if (*code == '\n')
        {
            ++line;
            col = 0;
        }
    }

    assert(codePos == code);

    ScriptPosition pos;
    pos.line = line;
    pos.column = col;

    return pos;
}
