// Header file for defining methods structures in ParserInterp.cpp 
parserInterp.h
#ifndef PARSERINTERP_H_
#define PARSERINTERP_H_

#include <iostream>
#include "lex.h"

using namespace std;

extern bool ProcName(istream& in, int& line);
extern bool Prog(istream& in, int& line);
extern bool ProcBody(istream& in, int& line);
extern bool DeclPart(istream& in, int& line);
extern bool DeclStmt(istream& in, int& line);
extern bool Type(istream& in, int& line);
extern bool StmtList(istream& in, int& line);
extern bool Stmt(istream& in, int& line);
extern bool PrintStmts(istream& in, int& line);
extern bool GetStmt(istream& in, int& line);
extern bool IfStmt(istream& in, int& line);
extern bool AssignStmt(istream& in, int& line);
extern bool Var(istream& in, int& line);
extern bool Expr(istream& in, int& line);
extern bool Relation(istream& in, int& line);
extern bool SimpleExpr(istream& in, int& line);
extern bool STerm(istream& in, int& line);
extern bool Term(istream& in, int& line);
extern bool Factor(istream& in, int& line);
extern bool Primary(istream& in, int& line);
extern bool Name(istream& in, int& line);
extern bool Range(istream& in, int& line);

extern int ErrCount();
#endif