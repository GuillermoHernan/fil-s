/// <summary>
/// Classes to handle source and object code positions.
/// </summary>

#pragma once

#include <string>

/// <summary>
///  Indicates a position inside a script file (line / column)
/// </summary>
struct ScriptPosition
{
    int line;
    int column;

    ScriptPosition() :
    line(-1), column(-1)
    {
    }

    ScriptPosition(int l, int c) :
    line(l), column(c)
    {
    }

    std::string toString()const;
    
    bool operator < (const ScriptPosition& b)const
    {
        if (line < b.line)
            return true;
        else if (line == b.line)
            return column < b.column;
        else
            return false;
    }
    
    bool operator == (const ScriptPosition& b)const
    {
        return (line == b.line && column == b.column);
    }
    
    bool operator > (const ScriptPosition& b)const
    {
        return !(*this <= b);
    }
    
    bool operator <= (const ScriptPosition& b)const
    {
        return (*this < b || *this == b);
    }
    
    bool operator >= (const ScriptPosition& b)const
    {
        return !(*this < b);
    }
	bool operator != (const ScriptPosition& b)const
	{
		return !(*this == b);
	}
};//struct ScriptPosition
