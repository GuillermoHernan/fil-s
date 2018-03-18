/// <summary>
/// Miscellaneous functions. 
/// </summary>

#pragma once

#include <string>
#include <sstream>
#include <stdarg.h>
#include <vector>

//#include "jsLexer.h"

typedef std::vector<std::string> StringVector;


bool isWhitespace(char ch);

bool isNumeric(char ch);
bool isNumber(const std::string &str);
bool isHexadecimal(char ch);
bool isOctal(char ch);
bool isOctal(const std::string& str);
bool isAlpha(char ch);
bool isAlphaNum(const std::string &str);
bool isIDString(const char *s);
void replace(std::string &str, char textFrom, const char *textTo);

bool startsWith(const std::string& str, const std::string& prefix);
StringVector split(const std::string& str, const std::string& separator);
std::string join(const StringVector& strings, const std::string& separator, size_t firstLine = 0);
std::string trim(const std::string& inputStr, const std::string& trimChars = " \t\n\r");

int copyWhile(char* dest, const char* src, bool(*conditionFN)(char), int maxLen);

const char* skipWhitespace(const char* input);
const char* skipNumeric(const char* input);
const char* skipHexadecimal(const char* input);

/// convert the given string into a quoted string suitable for javascript
std::string escapeString(const std::string &str, bool quote = true);

std::string indentText(int indent);

std::string double_to_string(double x);

double getNaN();

std::string readTextFile(const std::string& szPath);
bool writeTextFile(const std::string& szPath, const std::string& szContent);
bool createDirIfNotExist(const std::string& szPath);

std::string dirFromPath(const std::string& szPath);
std::string parentPath(const std::string& szPath);
std::string removeExt(const std::string& szPath);
std::string fileFromPath(const std::string& szPath);
std::string normalizePath(const std::string& path);
std::string joinPaths(const std::string& base, const std::string& relative);
bool isPathRelative(const std::string& path);

std::string getCurrentDirectory();

/// <summary>
/// Gets compile-time size of a static array.
/// </summary>
template<class T, size_t N>
constexpr size_t arrCount(T(&arr)[N])
{
    return N;
}


