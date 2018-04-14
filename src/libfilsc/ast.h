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
    AST_MODULE
    , AST_SCRIPT
    , AST_TYPEDEF
    , AST_LIST
    , AST_BLOCK
    , AST_TUPLE
    , AST_DECLARATION
    , AST_TUPLE_DEF
    , AST_TUPLE_ADAPTER
    , AST_IF
    , AST_FOR
    , AST_FOR_EACH
    , AST_RETURN
    , AST_FUNCTION
    , AST_FUNCTION_TYPE
    , AST_ASSIGNMENT
    , AST_FNCALL
    , AST_INTEGER
    , AST_FLOAT
    , AST_STRING
    , AST_BOOL
    , AST_IDENTIFIER
    , AST_ARRAY
    , AST_ARRAY_ACCESS
    , AST_MEMBER_ACCESS
    , AST_MEMBER_NAME
    , AST_BINARYOP
    , AST_PREFIXOP
    , AST_POSTFIXOP
    , AST_ACTOR
    , AST_DEFAULT_TYPE
    , AST_TYPE_NAME
    , AST_INPUT
    , AST_MESSAGE_TYPE
    , AST_OUTPUT
    , AST_UNNAMED_INPUT
    , AST_IMPORT
    , AST_GET_ADDRESS
    , AST_ARRAY_DECL

    //Remember to add new entries to 'astTypeToString' and 'astTypeFromString' functions!

    , AST_TYPES_COUNT
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
    ASTF_ACTOR_MEMBER = 8,
    ASTF_EXTERN_C = 16,
};

class AstNode;
typedef std::vector <Ref<AstNode> >				AstNodeList;
typedef std::map<std::string, Ref<AstNode>>		AstStr2NodesMap;

std::string astTypeToString(AstNodeTypes type);
AstNodeTypes astTypeFromString(const std::string& str);

AstNode*		astGetVoid();
AstNode*		astGetBool();
AstNode*		astGetInt();
AstNode*		astGetCPointer();

std::string		astTypeToString(AstNode* typeNode);
AstNode*		astGetParameters(AstNode* node);
AstNode*		astGetReturnType(AstNode* node);
AstNode*		astGetFunctionBody(AstNode* node);

bool			astIsTupleType(const AstNode* node);
bool			astCanBeCalled(const AstNode* node);
bool			astIsBoolType(const AstNode* type);
bool			astIsIntType(const AstNode* type);
bool			astIsCpointer(const AstNode* type);
bool			astIsVoidType(const AstNode* type);
bool            astIsDataType(const AstNode* node);

int				astFindMemberByName(AstNode* node, const std::string& name);

//Constructor functions
Ref<AstNode> astCreateModule(const std::string& name);
Ref<AstNode> astCreateScript(ScriptPosition pos, const std::string& name);
Ref<AstNode> astCreateTypedef(ScriptPosition pos, const std::string& name, Ref<AstNode> typeDesc);
Ref<AstNode> astCreateDeclaration(LexToken token,
    Ref<AstNode> typeDesc,
    Ref<AstNode> initExpr);
Ref<AstNode> astCreateDeclaration(ScriptPosition pos,
    const std::string& name,
    Ref<AstNode> typeDesc,
    Ref<AstNode> initExpr);
Ref<AstNode> astCreateArrayDecl(ScriptPosition pos, 
    Ref<AstNode> typeSpec,
    Ref<AstNode> sizeExpr);


Ref<AstNode> astCreateFunction(ScriptPosition pos,
    const std::string& name,
    Ref<AstNode> params,
    Ref<AstNode> returnType,
    Ref<AstNode> bodyExpr);
Ref<AstNode> astCreateFunctionType(ScriptPosition pos,
    Ref<AstNode> params,
    Ref<AstNode> returnType);


Ref<AstNode> astCreateBlock(LexToken token);
Ref<AstNode> astCreateTuple(ScriptPosition pos);
Ref<AstNode> astCreateTupleDef(ScriptPosition pos, const std::string& name);
Ref<AstNode> astCreateTupleAdapter(Ref<AstNode> tupleNode);
Ref<AstNode> astCreateIf(ScriptPosition pos,
    Ref<AstNode> condition,
    Ref<AstNode> thenSt,
    Ref<AstNode> elseSt);
Ref<AstNode> astCreateFor(ScriptPosition pos,
    Ref<AstNode> initSt,
    Ref<AstNode> condition,
    Ref<AstNode> incrementSt,
    Ref<AstNode> body);
Ref<AstNode> astCreateForEach(ScriptPosition pos,
    Ref<AstNode> itemDeclaration,
    Ref<AstNode> sequenceExpr,
    Ref<AstNode> body);
Ref<AstNode> astCreateReturn(ScriptPosition pos, Ref<AstNode> expr);
Ref<AstNode> astCreateAssignment(LexToken opToken,
    Ref<AstNode> lexpr,
    Ref<AstNode> rexpr);
Ref<AstNode> astCreatePrefixOp(LexToken token, Ref<AstNode> rexpr);
Ref<AstNode> astCreatePostfixOp(LexToken token, Ref<AstNode> lexpr);
Ref<AstNode> astCreateBinaryOp(LexToken token,
    Ref<AstNode> lexpr,
    Ref<AstNode> rexpr);
Ref<AstNode> astCreateFnCall(ScriptPosition pos, Ref<AstNode> fnExpr, Ref<AstNode> params);
Ref<AstNode> astCreateArray(ScriptPosition pos);
Ref<AstNode> astCreateArrayAccess(ScriptPosition pos,
    Ref<AstNode> arrayExpr,
    Ref<AstNode> indexExpr);
Ref<AstNode> astCreateMemberAccess(ScriptPosition pos,
    Ref<AstNode> objExpr,
    Ref<AstNode> identifier);

Ref<AstNode> astCreateActor(ScriptPosition pos, const std::string& name);

Ref<AstNode> astCreateInputMsg(ScriptPosition pos, const std::string& name);
Ref<AstNode> astCreateMessageType(ScriptPosition pos, Ref<AstNode> params);
Ref<AstNode> astCreateOutputMsg(ScriptPosition pos, const std::string& name);
Ref<AstNode> astCreateLiteral(LexToken token);
Ref<AstNode> astCreateBool(ScriptPosition pos, bool value);
//Ref<AstNode> astCreateDefaultType(Ref<DefaultType> type);
Ref<AstNode> astCreateUnnamedInput(ScriptPosition pos,
    Ref<AstNode> outputPath,
    Ref<AstNode> params,
    Ref<AstNode> code);
Ref<AstNode> astCreateImport(ScriptPosition pos, const std::string& value, int flags);

Ref<AstNode> astCreateGetAddress(ScriptPosition pos, Ref<AstNode> rExpr);


std::vector<AstNode*> astGatherTypes(Ref<AstNode> root);
std::vector<AstNode*> astGatherFunctions(AstNode* root);
std::vector<AstNode*> astGatherActors(AstNode* root);
std::vector<AstNode*> astGatherAll(AstNode* root);

class AstSerializeContext;

/// <summary>
/// Abstract syntax tree node class. 
/// These nodes form a tree which is the internal representation of the language from the 
/// parsing phase onwards.
/// </summary>
class AstNode : public RefCountObj
{
public:

    const AstNodeList& children()const
    {
        return m_children;
    }

    const std::string& getName()const
    {
        return m_name;
    }

    void setName(const std::string& name)
    {
        m_name = name;
    }

    const std::string& getValue()const
    {
        return m_value;
    }

    void addChild(Ref<AstNode> child)
    {
        m_children.push_back(child);
    }

    void setChild(unsigned index, Ref<AstNode> node)
    {
        assert(index < m_children.size());
        m_children[index] = node;
    }

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

    AstNodeTypes getType()const
    {
        return m_type;
    }

    void changeType(AstNodeTypes type)
    {
        m_type = type;
    }

    AstNode* getDataType()const;
    void setDataType(AstNode* dataType);

    AstNode* getReference()const
    {
        return m_reference;
    }
    void setReference(AstNode* node);

    int addFlag(AstFlags flag)
    {
        m_flags |= flag;
        return m_flags;
    }

    int addFlags(int flags)
    {
        m_flags |= flags;
        return m_flags;
    }

    bool hasFlag(AstFlags flag)const
    {
        return (m_flags & flag) != 0;
    }

    int getFlags()const
    {
        return m_flags;
    }

    //Ref<AstNode> findChildByName(const std::string& name);
    static int nodeCount()
    {
        return ms_nodeCount;
    }

    static Ref<AstNode> create(
        AstNodeTypes type,
        ScriptPosition pos,
        const std::string& name = "",
        const std::string& value = "",
        int flags = 0
    );

protected:
    AstNode(AstNodeTypes type,
        const ScriptPosition& pos,
        const std::string& name,
        const std::string& value,
        int flags);

    virtual ~AstNode()
    {
        --ms_nodeCount;
    }

private:
    const ScriptPosition	m_position;
    std::string				m_name;
    std::string				m_value;
    AstNodeList				m_children;

    //Reference for other node. On most nodes, it is its data type. On 'AST_IDENTIFIER',
    //it is the referenced declaration.
    AstNode*				m_reference;
    int						m_flags = 0;
    AstNodeTypes			m_type;

    static int ms_nodeCount;
};

#endif	/* AST_H */

