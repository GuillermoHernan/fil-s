/// <summary>
/// Abstract Syntax Tree classes / functions
/// </summary>

#ifndef AST_H
#define	AST_H

#pragma once

#include "RefCountObj.h"
#include "scriptPosition.h"
#include "lexer.h"

class SymbolScope;
class BaseType;
class DefaultType;

/**
 * AST node types enumeration
 */
enum AstNodeTypes
{
    AST_SCRIPT
	,AST_TYPEDEF
	,AST_LIST
    ,AST_BLOCK
	,AST_TUPLE
	,AST_DECLARATION
	,AST_TUPLE_DEF
    ,AST_IF
    ,AST_FOR
    ,AST_FOR_EACH
    ,AST_RETURN
    ,AST_FUNCTION
    ,AST_ASSIGNMENT
    ,AST_FNCALL
    //,AST_NEWCALL
	, AST_INTEGER
	, AST_FLOAT
	, AST_STRING
	,AST_BOOL
	,AST_IDENTIFIER
    ,AST_ARRAY
    ,AST_ARRAY_ACCESS
    ,AST_MEMBER_ACCESS
	,AST_MEMBER_NAME
    ,AST_BINARYOP
    ,AST_PREFIXOP
    ,AST_POSTFIXOP
    ,AST_ACTOR
    ,AST_CONNECT
	,AST_DEFAULT_TYPE
	,AST_TYPE_NAME

	,AST_TYPES_COUNT
};

/// <summary>
/// Enumeration with all possible AST node flags
/// </summary>
enum AstFlags
{
	ASTF_NONE = 0,
	ASTF_FUNCTION_PARAMETER = 1,
	ASTF_CONST = 2,
	ASTF_VAR = 4,
};

class AstNode;
class AstFunction;
class JSArray;
typedef std::vector <Ref<AstNode> > AstNodeList;

//AST conversion functions (mainly for AST /parser debugging)
Ref<JSArray> toJSArray (const AstNodeList& statements);
std::string toJSON (const AstNodeList& statements);

std::string astTypeToString(AstNodeTypes type);

//Constructor functions
Ref<AstNode> astCreateScript(ScriptPosition pos);
Ref<AstNode> astCreateTypedef(ScriptPosition pos, const std::string& name, Ref<AstNode> typeDesc);
Ref<AstNode> astCreateDeclaration(LexToken token,
	Ref<AstNode> typeDesc,
	Ref<AstNode> initExpr);
Ref<AstNode> astCreateDeclaration(ScriptPosition pos, 
	const std::string& name,
	Ref<AstNode> typeDesc,
	Ref<AstNode> initExpr);

Ref<AstNode> astCreateFunction(ScriptPosition pos, 
	const std::string& name, 
	Ref<AstNode> params, 
	Ref<AstNode> returnType, 
	Ref<AstNode> bodyExpr);

Ref<AstNode> astCreateBlock(LexToken token);
Ref<AstNode> astCreateTuple(LexToken token);
Ref<AstNode> astCreateIf (ScriptPosition pos,
                          Ref<AstNode> condition,
                          Ref<AstNode> thenSt,
                          Ref<AstNode> elseSt);
Ref<AstNode> astCreateConditional ( ScriptPosition pos, 
                                    Ref<AstNode> condition,
                                    Ref<AstNode> thenExpr,
                                    Ref<AstNode> elseExpr);
Ref<AstNode> astCreateFor (ScriptPosition pos, 
                          Ref<AstNode> initSt,
                          Ref<AstNode> condition,
                          Ref<AstNode> incrementSt,
                          Ref<AstNode> body);
Ref<AstNode> astCreateForEach (ScriptPosition pos, 
                          Ref<AstNode> itemDeclaration,
                          Ref<AstNode> sequenceExpr,
                          Ref<AstNode> body);
Ref<AstNode> astCreateReturn (ScriptPosition pos, Ref<AstNode> expr);
Ref<AstNode> astCreateAssignment(LexToken opToken,
                                 Ref<AstNode> lexpr, 
                                 Ref<AstNode> rexpr);
Ref<AstNode> astCreatePrefixOp(LexToken token, Ref<AstNode> rexpr);
Ref<AstNode> astCreatePostfixOp(LexToken token, Ref<AstNode> lexpr);
Ref<AstNode> astCreateBinaryOp(LexToken token, 
                                 Ref<AstNode> lexpr, 
                                 Ref<AstNode> rexpr);
Ref<AstNode> astCreateFnCall(ScriptPosition pos, Ref<AstNode> fnExpr, Ref<AstNode> params);
//Ref<AstNode> astToNewCall(Ref<AstNode> callExpr);
Ref<AstNode> astCreateArray(ScriptPosition pos);
Ref<AstNode> astCreateArrayAccess(ScriptPosition pos,
                                  Ref<AstNode> arrayExpr, 
                                  Ref<AstNode> indexExpr);
Ref<AstNode> astCreateMemberAccess(ScriptPosition pos,
                                  Ref<AstNode> objExpr, 
                                  Ref<AstNode> identifier);

Ref<AstFunction> astCreateInputMessage(ScriptPosition pos, const std::string& name);
Ref<AstFunction> astCreateOutputMessage(ScriptPosition pos, const std::string& name);
Ref<AstNode> astCreateConnect(ScriptPosition pos,
                                 Ref<AstNode> lexpr, 
                                 Ref<AstNode> rexpr);
Ref<AstNode> astCreateSend (ScriptPosition pos,
                                 Ref<AstNode> lexpr, 
                                 Ref<AstNode> rexpr);

Ref<AstNode> astCreateExtends (ScriptPosition pos,
                                const std::string& parentName);
Ref<AstNode> astCreateExport (ScriptPosition pos, Ref<AstNode> child);
Ref<AstNode> astCreateImport (ScriptPosition pos, Ref<AstNode> param);

Ref<AstNode> astGetExtends(Ref<AstNode> node);

Ref<AstNode> astCreateDefaultType(Ref<DefaultType> type);

/**
 * Base class for AST nodes
 */
class AstNode : public RefCountObj
{
public:

    virtual const AstNodeList& children()const
    {
        return ms_noChildren;
    }

	virtual const std::string getName()const
	{
		return "";
	}

	virtual void setName(const std::string& name)
	{
		assert(!"setName unsupported");
	}

    virtual std::string getValue()const
    {
        return "";
    }
    
	virtual void addChild(Ref<AstNode> child)
	{
		assert(!"addChildren unsupported");
	}

	virtual void setChild(unsigned index, Ref<AstNode> node)
	{
		assert(!"setChild unsupported");
	}

	virtual void destroy();

    bool childExists(size_t index)const
    {
        const AstNodeList&  c = children();
        
        if (index < c.size())
            return c[index].notNull();
        else
            return false;
    }

	Ref<AstNode> child(size_t index)const
	{
		const AstNodeList&  c = children();

		if (index < c.size())
			return c[index];
		else
			return Ref<AstNode>();
	}

	size_t childCount()const
	{
		return children().size();
	}

    const ScriptPosition& position()const
    {
        return m_position;
    }

    //virtual ASValue toJS()const;

    AstNodeTypes getType()const
    {
        return m_type;
    }

	void changeType(AstNodeTypes type)
	{
		m_type = type;
	}

	Ref<SymbolScope> getScope()const;
	void setScope(Ref<SymbolScope> scope);

	Ref<BaseType> getDataType()const;
	void setDataType(Ref<BaseType> dataType);

	int addFlag(AstFlags flag)
	{
		m_flags |= flag;
		return m_flags;
	}

	bool hasFlag(AstFlags flag)const
	{
		return (m_flags & flag) != 0;
	}

	//Ref<AstNode> findChildByName(const std::string& name);
	static int nodeCount()
	{
		return ms_nodeCount;
	}

protected:
    static const AstNodeList    ms_noChildren;
    
    const ScriptPosition m_position;
    AstNodeTypes m_type;

	AstNode(AstNodeTypes type, const ScriptPosition& pos);

    virtual ~AstNode()
    {
		--ms_nodeCount;
    }

private:
	Ref<RefCountObj>	m_scope;
	Ref<RefCountObj>	m_dataType;
	int					m_flags = 0;

	static int ms_nodeCount;
};

/**
 * Base class for AST nodes which contain children nodes.
 */
class AstBranchNode : public AstNode
{
public:

    virtual const AstNodeList& children()const
    {
        return m_children;
    }
    
    virtual void addChild(Ref<AstNode> child)
    {
        m_children.push_back(child);
    }

	virtual void setChild(unsigned index, Ref<AstNode> node)
	{
		assert(index < m_children.size());
		m_children[index] = node;
	}

    
    AstBranchNode(AstNodeTypes type, const ScriptPosition& pos) : AstNode(type, pos)
    {
    }
protected:
    
    AstNodeList     m_children;
    
};

/**
 * Class for branch nodes which are also named
 */
class AstNamedBranch : public AstBranchNode
{
public:

    virtual const std::string getName()const
    {
        return m_name;
    }

	virtual void setName(const std::string& name)
	{
		m_name = name;
	}

    AstNamedBranch(AstNodeTypes type, const ScriptPosition& pos, const std::string& _name)
    : AstBranchNode(type, pos), m_name(_name)
    {
    }
    
protected:

    std::string m_name;
};

/**
 * AST node for actor definitions
 */
class AstActor : public AstNamedBranch
{
public:
    static Ref<AstActor> create(ScriptPosition position,
                                   const std::string& name)
    {
        return refFromNew (new AstActor(position, name));
    }

protected:
    AstActor(ScriptPosition position, const std::string& name) :
    AstNamedBranch(AST_ACTOR, position, name)
    {
    }
};


/**
 * Base class for all AST nodes which represent operators.
 */
class AstOperator : public AstBranchNode
{
public:
    const std::string operation;
    
    //virtual ASValue toJS()const;

	virtual std::string getValue()const
	{
		return operation;
	}

    
    AstOperator (AstNodeTypes type, ScriptPosition position, const std::string& opText) : 
    AstBranchNode (type, position), operation(opText)
    {
    }
};

/**
 * AST node for primitive types literals (Number, String, Boolean)
 */
class AstLiteral : public AstNode
{
public:
    static Ref<AstLiteral> create(LexToken token);
	static Ref<AstLiteral> create(ScriptPosition pos, int value);
	static Ref<AstLiteral> createBool(ScriptPosition pos, bool value);
	static Ref<AstLiteral> createNull(ScriptPosition pos);
    

	virtual std::string getValue()const
	{
		return m_strValue;
	}

protected:
    AstLiteral (ScriptPosition position, AstNodeTypes type) : 
    AstNode(type, position)
    {
    }

private:
	std::string			m_strValue;
};

/**
 * AST node for identifiers (variable names, function names, members...)
 */
class AstIdentifier : public AstNode
{
public:
    static Ref<AstIdentifier> create(LexToken token)
    {
        return refFromNew(new AstIdentifier(token));
    }
    
    virtual const std::string getName()const
    {
        return m_name;
    }

	virtual void setName(const std::string& name)
	{
		m_name = name;
	}

	virtual std::string getValue()const
	{
		return m_name;
	}

protected:
    AstIdentifier (LexToken token) : 
    AstNode(AST_IDENTIFIER, token.getPosition()),
        m_name (token.text())
    {        
    }

    std::string   m_name;
};

#endif	/* AST_H */

