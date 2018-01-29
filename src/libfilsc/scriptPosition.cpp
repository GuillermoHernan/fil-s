/// <summary>
/// Classes to handle source and object code positions.
/// </summary>

#include "pch.h"
#include "ScriptPosition.h"

using namespace std;

/**
 * String representation of a Script position.
 * @return 
 */
string ScriptPosition::toString()const
{
    char buffer [128];

    sprintf_s(buffer, "(line: %d, col: %d): ", this->line, this->column);
    return string(buffer);
}
