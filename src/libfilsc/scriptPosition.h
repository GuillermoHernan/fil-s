/// <summary>
/// Classes to handle source and object code positions.
/// </summary>

#pragma once

#include <string>

/// <summary>
///  Indicates a position inside a script file (line / column)
/// </summary>
class ScriptPosition
{
public:
    ScriptPosition() :
    m_line(-1), m_column(-1)
    {
    }

    ScriptPosition(int l, int c) :
    m_line(l), m_column(c)
    {
    }

	int line()const		{ return m_line; }
	int column()const	{ return m_column; }

    std::string toString()const;
    
    bool operator < (const ScriptPosition& b)const
    {
        if (line() < b.line())
            return true;
        else if (line() == b.line())
            return column() < b.column();
        else
            return false;
    }
    
    bool operator == (const ScriptPosition& b)const
    {
        return (line() == b.line() && column() == b.column());
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

private:
	int m_line;
	int m_column;

};//struct ScriptPosition
