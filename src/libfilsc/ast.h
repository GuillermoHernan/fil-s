/// <summary>
/// Abstract Syntax Tree classes / functions
/// </summary>

#ifndef AST_H
#define	AST_H

#pragma once

#include "RefCountObj.h"
#include "scriptPosition.h"
#include "lexer.h"

/**
 * AST node types enumeration
 */
enum AstNodeTypes
{
    AST_SCRIPT
	,AST_LIST
    ,AST_BLOCK
	,AST_TUPLE
	,AST_DECLARATION
    ,AST_VAR
    ,AST_CONST
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
    ,AST_OBJECT
    ,AST_ARRAY_ACCESS
    ,AST_MEMBER_ACCESS
    ,AST_CONDITIONAL
    ,AST_BINARYOP
    ,AST_PREFIXOP
    ,AST_POSTFIXOP
    ,AST_ACTOR
    ,AST_CONNECT
    ,AST_INPUT
    ,AST_OUTPUT
    ,AST_CLASS
    ,AST_EXTENDS
    ,AST_EXPORT
    ,AST_IMPORT
    ,AST_TYPES_COUNT
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
Ref<AstNode> astCreateDeclaration(LexToken token,
	Ref<AstNode> typeDesc,
	Ref<AstNode> initExpr);

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
Ref<AstNode> astCreateVar (const ScriptPosition& pos, 
                           const std::string& name, 
                           Ref<AstNode> expr,
                           bool isConst);

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
    
    //virtual ASValue getValue()const
    //{
    //    return jsNull();
    //}
    
    virtual void addChild(Ref<AstNode> child)
    {
        assert(!"addChildren unsupported");
    }
    
    virtual void addParam(const std::string& paramName)
    {
        assert(!"addParam unsupported");
    }
    
    typedef std::vector<std::string> Params;
    virtual const Params& getParams()const
    {
        static const Params noParams;
        return noParams;
    }


    bool childExists(size_t index)const
    {
        const AstNodeList&  c = children();
        
        if (index < c.size())
            return c[index].notNull();
        else
            return false;
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

protected:
    static const AstNodeList    ms_noChildren;
    
    const ScriptPosition m_position;
    AstNodeTypes m_type;

    AstNode(AstNodeTypes type, const ScriptPosition& pos) :
    m_position(pos), m_type(type)
    {
    }

    virtual ~AstNode()
    {
    }
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

    AstNamedBranch(AstNodeTypes type, const ScriptPosition& pos, const std::string& _name)
    : AstBranchNode(type, pos), m_name(_name)
    {
    }
    
protected:

    const std::string m_name;
};

/**
 * AST node for function definitions
 */
class AstFunction : public AstNode
{
public:
    static Ref<AstFunction> create(ScriptPosition position,
                                   const std::string& name)
    {
        return refFromNew (new AstFunction(AST_FUNCTION, position, name));
    }

    void setCode(Ref<AstNode> code)
    {
        m_code = code;
    }

    Ref<AstNode> getCode()const
    {
        return m_code;
    }
    
    virtual const std::string getName()const
    {
        return m_name;
    }

    virtual void addParam(const std::string& paramName)
    {
        m_params.push_back(paramName);
    }

    virtual const Params& getParams()const
    {
        return m_params;
    }
    
    ////virtual ASValue toJS()const;

    AstFunction(AstNodeTypes type, ScriptPosition position, const std::string& name) :
    AstNode(type, position),
    m_name(name)
    {
    }
    
protected:

    const std::string   m_name;
    Params              m_params;
    Ref<AstNode>        m_code;
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

    virtual void addParam(const std::string& paramName)
    {
        m_params.push_back(paramName);
    }

    virtual const Params& getParams()const
    {
        return m_params;
    }
    
    //virtual ASValue toJS()const;

protected:
    AstActor(ScriptPosition position, const std::string& name) :
    AstNamedBranch(AST_ACTOR, position, name)
    {
    }
    
    Params              m_params;
};


/**
 * Base class for all AST nodes which represent operators.
 */
class AstOperator : public AstBranchNode
{
public:
    const std::string operation;
    
    //virtual ASValue toJS()const;
    
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

protected:
    AstIdentifier (LexToken token) : 
    AstNode(AST_IDENTIFIER, token.getPosition()),
        m_name (token.text())
    {        
    }

    const std::string   m_name;
};

/**
 * AST node for object literals.
 */
class AstObject : public AstNode
{
public:
    /**
     * Object property structure
     */
    struct Property
    {
        std::string     name;
        Ref<AstNode>    expr;
        bool            isConst;
    };
    typedef std::vector<Property>   PropertyList;
    
    static Ref<AstObject> create(ScriptPosition pos)
    {
        return refFromNew (new AstObject(pos));
    }
    
    void addProperty (const std::string name, Ref<AstNode> expr, bool isConst)
    {
        Property prop;
        prop.name = name;
        prop.expr = expr;
        prop.isConst = isConst;
        
        m_properties.push_back(prop);
    }
    
    const PropertyList & getProperties()const
    {
        return m_properties;
    }
    
    //virtual ASValue toJS()const;

protected:
    AstObject(ScriptPosition pos) : AstNode(AST_OBJECT, pos)
    {
    }
    
    PropertyList m_properties;
};


/**
 * AST node for class definitions
 */
class AstClassNode : public AstNamedBranch
{
public:
    static Ref<AstClassNode> create(ScriptPosition position,
                                   const std::string& name)
    {
        return refFromNew (new AstClassNode(position, name));
    }

    virtual void addParam(const std::string& paramName)
    {
        m_params.push_back(paramName);
    }

    virtual const Params& getParams()const
    {
        return m_params;
    }
    
    Ref<AstNamedBranch> getExtendsNode()const;
    
    //virtual ASValue toJS()const;

protected:
    AstClassNode(ScriptPosition position, const std::string& name) :
    AstNamedBranch(AST_CLASS, position, name)
    {
    }
    
    Params              m_params;
};


#endif	/* AST_H */

