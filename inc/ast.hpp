#ifndef __AST_HPP__
#define __AST_HPP__
#include<common.hpp>
#include<tokenize.hpp>
typedef enum{
    ND_RETURN,
    ND_NUM,
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_MOD,
    ND_EXPR_STMT, // 表达式语句
    ND_STMT_EXPR, // 语句表达式
}NodeKind;

class ASTNode
{
protected:
    NodeKind Kind;
    Token *Tok;
public:
    ASTNode *LHS = NULL,*RHS = NULL;
    ASTNode *Next = NULL;
    ~ASTNode(){};
    ASTNode(){}
    ASTNode(NodeKind Kind,Token *&Tok);
    void newBinary(NodeKind Kind, ASTNode *LHS, ASTNode *RHS);
    void newUnary(NodeKind Kind, ASTNode *LHS);
    void AddLHS(ASTNode *LHS);

    void AddRHS(ASTNode *RHS);
    int getKind();
    virtual int getVal(){
        if(this->Kind != ND_NUM){
            Log("ASTNode:%s",this->Tok->Name.c_str());
            error("This ASTNode don't have val.");
            
        }
        return 0;
    }
};
class NumNode : public ASTNode
{
private:
    union Val
    {
        float f;
        int   i;
    }val;
    
public:
    NumNode(Token *&Tok): ASTNode(ND_NUM, Tok){
        this->val.i = Tok->getVal();
        Tok = Tok->Next;
    }
    ~NumNode(){
    };
    int getVal()override{
        return this->val.i;
    }
};
//====================================================
//Root
class ObjNode
{
public:
    string Name;
    ObjNode *Next=NULL;
    bool IsFunc;
    ObjNode(Token *&Tok){
        this->Name = Tok->Name;
        Tok = Tok->Next;
    }
    // ~ObjNode();
    virtual void AddBody(ASTNode *Body){
        if(!this->IsFunc){
            error("Add function body error.");
        }
    }
    virtual void AddParams(ObjNode *Body){
        if(!this->IsFunc){
            error("Add function Params error.");
        }
    }
    virtual ASTNode *GetBody(){
        if(!this->IsFunc){
            error("This is not function.\nDon't have Body.");
        }
        return NULL;
    }
};

class FuncNode : public ObjNode
{
private:
    ASTNode *Body = NULL;
    ObjNode *Locals = NULL;
    ObjNode *Params = NULL;
    uint32_t StackSize = 0;

public:
    ~FuncNode();
    FuncNode(Token *&Tok);
    void AddBody(ASTNode *Body);
    void AddParams(ObjNode *Params);
    ASTNode *GetBody();
};
// class VarNode : public ObjNode
// {
// private:
//     string VarName;
//     uint32_t Offset;
//     bool IsLocal;
// public:
//     VarNode(/* args */);
//     ~VarNode();
// };
// class IFNode : public ASTNode
// {
// private:
//     ASTNode *Cond;
//     ASTNode *Then;
//     ASTNode *Els;
// };
// class ForNode : public IFNode
// {
// private:
//     ASTNode *Init;
//     ASTNode *Inc;
// public:
//     ForNode(/* args */);
//     ~ForNode();
// };


ObjNode *ASTBuild(TokenList *list);

#endif