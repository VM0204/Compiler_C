/* Implementation of Interpreter for the SADAL Language */
ParserInterp.cpp
#include <iostream>
#include <vector>
#include <sstream>
#include <queue>
#include <string>
#include "parserInterp.h"

// Global maps: Track declared variables, symbol table with types, runtime variable values, and temporary lists
map<string, bool> defVar;
map<string, Token> SymTable;
map<string, Value> TempsResults;

vector<string> *IdsList;
static Value LastDeclaredType;

// Parser namespace: manage token retrieval and pushback for lookahead
namespace Parser 
{
bool pushed_back = false;
LexItem pushed_token;

static LexItem GetNextToken(istream& in, int& line) 
{
if (pushed_back) 
{
pushed_back = false;
return pushed_token;
}
return getNextToken(in, line);
}

static void PushBackToken(LexItem& t) 
{
if (pushed_back) 
{
abort();
}
pushed_back = true;
pushed_token = t;
}
}

// Error handling: count errors and report type of parsing error
static int error_count = 0;

int ErrCount() 
{
return error_count;
}

void ParseError(int line, string msg) 
{
++error_count;
cout << line << ": " << msg << endl;
}

// Ensure program starts with PROCEDURE, validate procedure name and IS keyword, parse the body, end with DONE
bool Prog(istream& in, int& line) 
{
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != PROCEDURE) 
    {
        ParseError(1, "Incorrect compilation file.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != IDENT) 
    {
        ParseError(line, "Missing procedure name.");
        return false;
    }

    defVar[procName] = true;

    tok = Parser::GetNextToken(in, line);
    if (tok != IS) 
    {
        ParseError(line, "Missing IS keyword.");
        return false;
    }

    if (!ProcBody(in, line, procName))
        return false;

    tok = Parser::GetNextToken(in, line);
    if (tok != DONE) 
    {
        return true;
    }

    cout << "\n(DONE)" << endl;
    return true;
}

// Parse declaration part and statement list inside BEGIN-END. Verify that procedure name matches at the end
bool ProcBody(istream& in, int& line, const string& procName) 
{
    if (!DeclPart(in, line)) 
    {
        ParseError(line, "Non-recognizable Declaration Part.");
        ParseError(line + 1, "Incorrect compilation file.");
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != BEGIN) 
    {
        ParseError(line, "Incorrect Declaration Statement Syntax.");
        ParseError(line, "Non-recognizable Declaration Part.");
        ParseError(line + 1, "Incorrect compilation file.");
        return false;
    }

    if (!StmtList(in, line))
        return false;

    tok = Parser::GetNextToken(in, line);
    if (tok != END) 
    {
        return true;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != IDENT) 
    {
        ParseError(line, "Missing procedure end identifier.");
        return false;
    }

    if (tok.GetLexeme() != procName) 
    {
        ParseError(line, "Procedure name mismatch in closing end identifier.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) 
    {
        ParseError(line, "Missing semicolon at the end of the statement.");
        return false;
    }

    return true;
}

// Parse sequence of declarations until BEGIN keyword is found
bool DeclPart(istream& in, int& line) 
{
    bool status = DeclStmt(in, line);
    if (!status) return false;

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok == BEGIN) 
    {
        Parser::PushBackToken(tok);
        return true;
    } else 
    {
        Parser::PushBackToken(tok);
        return DeclPart(in, line);
    }

    ParseError(line, "Non-recognizable Declaration Part.");
    return false;
}

// Parse declaration statement with identifiers, type, optional initialization, and input into the symbol table
bool DeclStmt(istream& in, int& line) 
{
    IdsList = new vector<string>;
    LexItem tok = Parser::GetNextToken(in, line);

    if (tok != IDENT) 
    {
        ParseError(line, "Missing identifier.");
        delete IdsList;
        return false;
    }
    IdsList->push_back(tok.GetLexeme());

    while (true) 
    {
        tok = Parser::GetNextToken(in, line);
        if (tok == COMMA) 
        {
            tok = Parser::GetNextToken(in, line);
            if (tok != IDENT) 
            {
                ParseError(line, "Expected identifier after comma.");
                delete IdsList;
                return false;
            }
            IdsList->push_back(tok.GetLexeme());
        } else 
        {
            break;
        }
    }

    if (tok != COLON) 
    {
        ParseError(line, "Missing colon in declaration.");
        delete IdsList;
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != INT && tok != FLOAT && tok != BOOL && tok != STRING && tok != CHAR) 
    {
        ParseError(line, "Invalid type in declaration.");
        delete IdsList;
        return false;
    }

    Token varType = tok.GetToken();
    for (const auto& id : *IdsList) 
    {
        if (SymTable.find(id) != SymTable.end())
        {
            ParseError(line, "Redeclaration of variable " + id);
            delete IdsList;
            return false;
        }
        SymTable[id] = varType;
        defVar[id] = true;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok == ASSOP) 
    {
        Value val;
        if (!Expr(in, line, val)) 
        {
            delete IdsList;
            return false;
        }

        ValType exprType = val.GetType();
        bool typeMatch =
            (varType == INT && exprType == VINT) ||
            (varType == FLOAT && exprType == VREAL) ||
            (varType == BOOL && exprType == VBOOL) ||
            (varType == STRING && exprType == VSTRING) ||
            (varType == CHAR && exprType == VCHAR);

        if (!typeMatch) 
        {
            ParseError(line, "Run-Time Error - Illegal Assignment Operation");
            delete IdsList;
            return false;
        }

        for (auto id : *IdsList) 
        {
            TempsResults[id] = val;
        }

        tok = Parser::GetNextToken(in, line);
    }

    if (tok != SEMICOL) {
        ParseError(line, "Missing semicolon at end of declaration.");
        delete IdsList;
        return false;
    }

    delete IdsList;
    return true;
}

// Parse and execute list of statements until END/ELSE/ELSIF
bool StmtList(istream& in, int& line) 
{

    while (true) 
    {
        LexItem tok = Parser::GetNextToken(in, line);
        if (tok == END || tok == ELSE || tok == ELSIF) 
        {
            Parser::PushBackToken(tok);
            return true;
        }

        Parser::PushBackToken(tok);
        if (!Stmt(in, line)) 
        {
            ParseError(line, "Syntactic error in statement list.");
            return false;
        }
    }
}

// Parse one statement. Can be assignment, output, input, or if
bool Stmt(istream& in, int& line) 
{

    LexItem tok = Parser::GetNextToken(in, line);

    if (tok == IDENT) 
    {
        Parser::PushBackToken(tok);
        return AssignStmt(in, line);
    }
    else if (tok == PUTLN || tok == PUT) 
    {
        Parser::PushBackToken(tok);
        return PrintStmts(in, line);
    }
    else if (tok == GET) 
    {
        Parser::PushBackToken(tok);
        return GetStmt(in, line);
    }
    else if (tok == IF) 
    {
        Parser::PushBackToken(tok);
        return IfStmt(in, line);
    }
    else 
    {
        ParseError(line, "Invalid statement start.");
        return false;
    }
}

// Parse PUT/PUTLN statements and print evaluated expression
bool PrintStmts(istream& in, int& line) 
{

    LexItem tok = Parser::GetNextToken(in, line);
    bool newline = (tok == PUTLN);

    tok = Parser::GetNextToken(in, line);
    if (tok != LPAREN) 
    {
        ParseError(line, "Missing Left Parenthesis");
        ParseError(line, "Invalid put statement.");
        return false;
    }

    Value val;
    if (!Expr(in, line, val)) 
    {
        ParseError(line, "Missing expression for an output statement");
        ParseError(line, "Invalid put statement.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != RPAREN) 
    {
        ParseError(line, "Missing Right Parenthesis");
        ParseError(line, "Invalid put statement.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) 
    {
        ParseError(line-1, "Missing semicolon at end of statement");
        ParseError(line-1, "Invalid put statement.");
        return false;
    }

    cout << val;
    if (newline)
        cout << endl;

    return true;
}

// Parse GET statement. Read user input, convert to proper type, assign to variable with type checking
bool GetStmt(istream& in, int& line) 
{

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != GET) 
    {
        ParseError(line, "Invalid get statement.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != LPAREN) 
    {
        ParseError(line, "Missing opening parenthesis in get statement.");
        return false;
    }

    LexItem idTok;
    if (!Var(in, line, idTok))
        return false;

    string varName = idTok.GetLexeme();

    tok = Parser::GetNextToken(in, line);
    if (tok != RPAREN) 
    {
        ParseError(line, "Missing closing parenthesis in get statement.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) 
    {
        ParseError(line, "Missing semicolon.");
        return false;
    }

    string input;
    cin >> input;

    Token type = SymTable[varName];
    try 
    {
        if (type == INT) 
        {
            TempsResults[varName] = Value(stoi(input));
        }
        else if (type == FLOAT) 
        {
            TempsResults[varName] = Value(stod(input));
        }
        else if (type == BOOL) 
        {
            if (input == "true")
                TempsResults[varName] = Value(true);
            else if (input == "false")
                TempsResults[varName] = Value(false);
            else 
            {
                ParseError(line, "Invalid boolean input.");
                return false;
            }
        }
        else if (type == STRING) 
        {
            TempsResults[varName] = Value(input);
        }
        else if (type == CHAR) 
        {
            if (input.length() != 1) 
            {
                ParseError(line, "Invalid character input.");
                return false;
            }
            TempsResults[varName] = Value(input[0]);
        }
    }
    catch (const exception& e) 
    {
        ParseError(line, "Invalid input for variable type.");
        return false;
    }

    return true;
}

// Parse IF-THEN-ELSIF-ELSE-END IF structure. Evaluate conditions, execute only first true branch, skip others. Handle nesting
bool IfStmt(istream& in, int& line) 
{

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != IF) 
    {
        ParseError(line, "Invalid expression type for an If condition.");
        ParseError(line, "Invalid If statement.");
        return false;
    }

    Value condVal;
    if (!Expr(in, line, condVal))
        return false;

    if (condVal.GetType() != VBOOL) 
    {
        ParseError(line, "Missing if statement condition");
        ParseError(line, "Invalid If statement.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != THEN) 
    {
        ParseError(line, "Missing THEN in If statement");
        ParseError(line, "Invalid If statement.");
        return false;
    }

    bool condExecuted = false;
    if (condVal.GetBool()) 
    {
        condExecuted = true;
        if (!StmtList(in, line))
            return false;
    }
    else 
    {
        int nesting = 0;
        while (true) {
            tok = Parser::GetNextToken(in, line);
            if (tok == IF)
            {
                nesting++;
            }
            else if (tok == END && nesting > 0) 
            {
                nesting--;
            }
            else if ((tok == END || tok == ELSIF || tok == ELSE) && nesting == 0) 
            {
                break;
            }
        }
    }

    while (tok == ELSIF) 
    {
        if (condExecuted) 
        {
            if (!Expr(in, line, condVal))
                return false;

            tok = Parser::GetNextToken(in, line);
            if (tok != THEN) 
            {
                ParseError(line, "Missing Statement for If-Stmt Then-clause");
                ParseError(line, "Invalid If statement.");
                return false;
            }

            int nesting = 0;
            while (true) 
            {
                tok = Parser::GetNextToken(in, line);
                if (tok == IF) {
                    nesting++;
                }
                else if (tok == END && nesting > 0) 
                {
                    nesting--;
                }
                else if ((tok == END || tok == ELSIF || tok == ELSE) && nesting == 0) 
                {
                    break;
                }
            }
        }
        else 
        {
            if (!Expr(in, line, condVal))
                return false;

            if (condVal.GetType() != VBOOL) 
            {
                ParseError(line, "Invalid expression type for an Elsif condition");
                ParseError(line, "Invalid If statement.");
                return false;
            }

            tok = Parser::GetNextToken(in, line);
            if (tok != THEN) 
            {
                ParseError(line, "Missing THEN in Elsif statement");
                ParseError(line, "Invalid If statement.");
                return false;
            }

            if (condVal.GetBool()) 
            {
                condExecuted = true;
                if (!StmtList(in, line))
                    return false;

                tok = Parser::GetNextToken(in, line);
                while (tok != END && tok != ELSIF && tok != ELSE) 
                {
                    tok = Parser::GetNextToken(in, line);
                }
            }
            else 
            {
                int nesting = 0;
                while (true) 
                {
                    tok = Parser::GetNextToken(in, line);
                    if (tok == IF) 
                    {
                        nesting++;
                    }
                    else if (tok == END && nesting > 0) 
                    {
                        nesting--;
                    }
                    else if ((tok == END || tok == ELSIF || tok == ELSE) && nesting == 0) 
                    {
                        break;
                    }
                }
            }
        }
    }

    if (tok == ELSE) 
    {
        if (condExecuted) 
        {
            int nesting = 0;
            while (true) {
                tok = Parser::GetNextToken(in, line);
                if (tok == IF) 
                {
                    nesting++;
                }
                else if (tok == END && nesting > 0) 
                {
                    nesting--;
                }
                else if (tok == END && nesting == 0) 
                {
                    break;
                }
            }
        }
        else 
        {
            if (!StmtList(in, line))
                return false;

            tok = Parser::GetNextToken(in, line);
        }
    }

    if (tok != END) 
    {
        ParseError(line, "Missing END in IF statement.");
        return true;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != IF) 
    {
        ParseError(line, "Missing IF after END in If statement");
        ParseError(line, "Invalid If statement.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) 
    {
        ParseError(line, "Missing closing END IF for If-statement.");
        ParseError(line, "Invalid If statement.");
        return false;
    }

    return true;
}

// Parse assignment statements. Evaluate RHS expression, checks type matching, update variable value
bool AssignStmt(istream& in, int& line) 
{

    LexItem idTok;
    if (!Var(in, line, idTok))
        return false;

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != ASSOP) 
    {
        ParseError(line, "Missing Assignment Operator");
        ParseError(line, "Invalid assignment statement.");
        return false;
    }

    Value val;
    if (!Expr(in, line, val))
        return false;

    string varName = idTok.GetLexeme();

    if (SymTable.find(varName) == SymTable.end()) 
    {
        ParseError(line, "Undeclared variable: " + varName);
        return false;
    }

    Token varType = SymTable[varName];
    ValType exprType = val.GetType();

    bool typeMatch = false;
    if ((varType == INT && exprType == VINT) ||
        (varType == FLOAT && exprType == VREAL) ||
        (varType == BOOL && exprType == VBOOL) ||
        (varType == STRING && exprType == VSTRING) ||
        (varType == CHAR && exprType == VCHAR)) 
        {
        typeMatch = true;
    }

    if (!typeMatch) 
    {
        ParseError(line, "Run-Time Error-Illegal Assignment Operation");
        return false;
    }

    TempsResults[varName] = val;

    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) 
    {
        ParseError(line, "Missing semicolon after assignment");
        return false;
    }

    return true;
}

// Parse logical expressions with AND/OR operators
bool Expr(istream& in, int& line, Value& retVal) 
{

    Value val1, val2;

    if (!Relation(in, line, val1))
        return false;

    LexItem tok = Parser::GetNextToken(in, line);
    while (tok == AND || tok == OR) 
    {
        if (!Relation(in, line, val2)) 
        {
            ParseError(line, "Missing operand after logical operator");
            return false;
        }
        try 
        {
            if (tok == AND)
                val1 = val1 && val2;
            else if (tok == OR)
                val1 = val1 || val2;
        }
        catch (const char* error) 
        {
            ParseError(line, error);
            return false;
        }

        tok = Parser::GetNextToken(in, line);
    }

    Parser::PushBackToken(tok);
    retVal = val1;
    return true;
}

// Parse relational expressions
bool Relation(istream& in, int& line, Value& retVal) 
{

    Value val1, val2;

    if (!SimpleExpr(in, line, val1))
        return false;

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok == EQ || tok == NEQ || tok == LTHAN || tok == LTE || tok == GTHAN || tok == GTE) 
    {
        if (!SimpleExpr(in, line, val2)) 
        {
            ParseError(line, "Missing operand after relational operator");
            return false;
        }
        try {
            if (tok == EQ)
                retVal = (val1 == val2);
            else if (tok == NEQ)
                retVal = (val1 != val2);
            else if (tok == LTHAN)
                retVal = (val1 < val2);
            else if (tok == LTE)
                retVal = (val1 <= val2);
            else if (tok == GTHAN)
                retVal = (val1 > val2);
            else if (tok == GTE)
                retVal = (val1 >= val2);
        }
        catch (const char* error) 
        {
            ParseError(line, error);
            return false;
        }
    }
    else 
    {
        Parser::PushBackToken(tok);
        retVal = val1;
    }

    return true;
}

// Parse addition, subtraction, concatenation operations
bool SimpleExpr(istream& in, int& line, Value& retVal) 
{

    Value val1, val2;

    if (!STerm(in, line, val1))
        return false;

    LexItem tok = Parser::GetNextToken(in, line);
    while (tok == PLUS || tok == MINUS || tok == CONCAT) 
    {
        if (!STerm(in, line, val2)) 
        {
            ParseError(line, "Missing operand after operator");
            return false;
        }
       
        try 
        {
            if (tok == PLUS)
                val1 = val1 + val2;
            else if (tok == MINUS)
                val1 = val1 - val2;
            else if (tok == CONCAT)
                val1 = val1.Concat(val2);
        }
        catch (const char* error) 
        {
            ParseError(line, error);
            return false;
        }

        tok = Parser::GetNextToken(in, line);
    }

    Parser::PushBackToken(tok);
    retVal = val1;
    return true;
}

// Parses signed terms
bool STerm(istream& in, int& line, Value& retVal) 
{

    LexItem tok = Parser::GetNextToken(in, line);
    int sign = 1;

    if (tok == PLUS || tok == MINUS) 
    {
        sign = (tok == PLUS) ? 1 : -1;
    }
    else 
    {
        Parser::PushBackToken(tok);
    }

    if (!Term(in, line, sign, retVal))
        return false;

    return true;
}

// Parse multiplication, division, modulus expressions
bool Term(istream& in, int& line, int sign, Value& retVal) 
{

    Value val1, val2;

    if (!Factor(in, line, sign, val1))
        return false;

    LexItem tok = Parser::GetNextToken(in, line);
    while (tok == MULT || tok == DIV || tok == MOD) 
    {
        int nextSign = 1;
        if (!Factor(in, line, nextSign, val2))
        {
            ParseError(line, "Missing operand after operator");
            return false;
        }
        try {
            if (tok == MULT)
                val1 = val1 * val2;
            else if (tok == DIV)
                val1 = val1 / val2;
            else if (tok == MOD)
                val1 = val1 % val2;
        }
        catch (const char* error) 
        {
            ParseError(line, error);
            return false;
        }

        tok = Parser::GetNextToken(in, line);
    }

   
   
    Parser::PushBackToken(tok);
    retVal = val1;
    return true;
}

// Parse NOT operator, exponentiation, or pass to Primary
bool Factor(istream& in, int& line, int sign, Value& retVal) 
{

    LexItem tok = Parser::GetNextToken(in, line);

    if (tok == NOT) 
    {
        Value val;
        if (!Factor(in, line, 1, val)) 
        {
            ParseError(line, "Incorrect operand");
            return false;
        }

        try 
        {
            retVal = !val;
        }
        catch (const char* error) 
        {
            ParseError(line, error);
            return false;
        }

        return true;
    }

    Parser::PushBackToken(tok);

    if (!Primary(in, line, sign, retVal))
        return false;

    tok = Parser::GetNextToken(in, line);
    if (tok == EXP) {
        Value exp;
        tok = Parser::GetNextToken(in, line);
        int expSign = 1;
        if (tok == PLUS || tok == MINUS) 
        {
            expSign = (tok == PLUS) ? 1 : -1;
        }
        else 
        {
            Parser::PushBackToken(tok);
        }

        if (!Primary(in, line, expSign, exp)) 
        {
            ParseError(line, "Missing exponent after ** operator");
            return false;
        }

        try 
        {
            retVal = retVal.Exp(exp);
        }
        catch (const char* error) 
        {
            ParseError(line, error);
            return false;
        }
    }
    else 
    {
        Parser::PushBackToken(tok);
    }

    return true;
}

// Handle constants, identifiers, or parenthesized sub-expressions
bool Primary(istream& in, int& line, int sign, Value& retVal) 
{

    LexItem tok = Parser::GetNextToken(in, line);

    if (tok == ICONST) 
    {
        int value = stoi(tok.GetLexeme());
        retVal = Value(sign * value);
        return true;
    }
    else if (tok == FCONST) 
    {
        double value = stod(tok.GetLexeme());
        retVal = Value(sign * value);
        return true;
    }
    else if (tok == SCONST) 
    {
        if (sign == -1) 
        {
            ParseError(line, "Run-Time Error-Illegal sign operation on string");
            return false;
        }
        retVal = Value(tok.GetLexeme());
        return true;
    }
    else if (tok == BCONST) 
    {
        if (sign == -1) 
        {
            ParseError(line, "Run-Time Error-Illegal sign operation on boolean");
            return false;
        }
        bool value = (tok.GetLexeme() == "true");
        retVal = Value(value);
        return true;
    }
    else if (tok == CCONST) 
    {
        if (sign == -1) {
            ParseError(line, "Run-Time Error-Illegal sign operation on character");
            return false;
        }
        char value = tok.GetLexeme()[0];
        retVal = Value(value);
        return true;
    }
    else if (tok == IDENT) 
    {
        Parser::PushBackToken(tok);
        if (!Name(in, line, sign, retVal))
            return false;
        return true;
    }
    else if (tok == LPAREN) 
    {
        Value val;
        if (!Expr(in, line, val))
            return false;

        tok = Parser::GetNextToken(in, line);
        if (tok != RPAREN) 
        {
            ParseError(line, "Missing right parenthesis after expression");
            return false;
        }
        if (val.IsInt())
            retVal = Value(sign * val.GetInt());
        else if (val.IsReal())
            retVal = Value(sign * val.GetReal());
        else if (sign == -1) 
        {
            ParseError(line, "Run-Time Error-Illegal sign operation");
            return false;
        }
        else
            retVal = val;

        return true;
    }
    else 
    {
        ParseError(line, "Invalid primary expression");
        return false;
    }
}

// Parse variable references and check declaration status
bool Var(istream& in, int& line, LexItem& idtok) 
{

    idtok = Parser::GetNextToken(in, line);

    if (idtok != IDENT) 
    {
        ParseError(line, "Missing Variable");
        return false;
    }

    string varName = idtok.GetLexeme();
    if (defVar.find(varName) == defVar.end() || !defVar[varName]) 
    {
        ParseError(line, "Undeclared Variable: " + varName);
        return false;
    }

    return true;
}

// Retrieve variable value from runtime table. Handle string indexing and slicing operations
bool Name(istream& in, int& line, int sign, Value& retVal) 
{

    LexItem idTok;
    if (!Var(in, line, idTok))
        return false;

    string varName = idTok.GetLexeme();
    if (TempsResults.find(varName) == TempsResults.end()) 
    {
        ParseError(line, "Run-Time Error-Using uninitialized variable" + varName);
        ParseError(line, "Invalid reference to a variable.");
        return false;
    }

    Value varValue = TempsResults[varName];
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok == LPAREN) {
        if (!varValue.IsString()) 
        {
            ParseError(line, "Run-Time Error-Indexing a non-string variable");
            return false;
        }
        Value index1, index2;
        if (!SimpleExpr(in, line, index1)) 
        {
            return false;
        }
        tok = Parser::GetNextToken(in, line);
        if (tok == DOT) 
        {
            tok = Parser::GetNextToken(in, line);
            if (tok != DOT) 
            {
                ParseError(line, "Missing second dot in range");
                return false;
            }

            if (!SimpleExpr(in, line, index2)) 
            {
                return false;
            }
            if (!index1.IsInt() || !index2.IsInt()) 
            {
                ParseError(line, "Run-Time Error-Non-integer index for string");
                return false;
            }

            int i1 = index1.GetInt();
            int i2 = index2.GetInt();
            string str = varValue.GetString();

            if (i1 < 0 || i1 >= str.length() || i2 < 0 || i2 >= str.length()) 
            {
                ParseError(line, "Run-Time Error-Index out of bounds");
                return false;
            }

            if (i1 > i2) 
            {
                ParseError(line, "Run-Time Error-Invalid range bounds");
                return false;
            }
            string substr = str.substr(i1, i2 - i1 + 1);
            retVal = Value(substr);
        }
        else 
        {
            Parser::PushBackToken(tok);
            if (!index1.IsInt()) 
            {
                ParseError(line, "Run-Time Error-Non-integer index for string");
                return false;
            }

            int idx = index1.GetInt();
            string str = varValue.GetString();

            if (idx < 0 || idx >= str.length()) 
            {
                ParseError(line, "Run-Time Error-Index out of bounds");
                return false;
            }
            retVal = Value(str[idx]);
        }

        tok = Parser::GetNextToken(in, line);
        if (tok != RPAREN) 
        {
            ParseError(line, "Missing right parenthesis after index");
            return false;
        }
    }
    else 
    {
        Parser::PushBackToken(tok);
        if (varValue.IsInt())
            retVal = Value(sign * varValue.GetInt());
        else if (varValue.IsReal())
            retVal = Value(sign * varValue.GetReal());
        else if (sign == -1) {
            ParseError(line, "Run-Time Error-Illegal sign operation");
            return false;
        }
        else
            retVal = varValue;
    }

    return true;
}

// Parse integer ranges while checking bounds
bool Range(istream& in, int& line, Value& retVal1, Value& retVal2) 
{

    if (!SimpleExpr(in, line, retVal1)) 
    {
        ParseError(line, "Invalid range syntax.");
        return false;
    }

    if (!retVal1.IsInt()) 
    {
        ParseError(line, "Run-Time Error-Non-integer index");
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != DOT) 
    {
        ParseError(line, "Missing dot in range");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != DOT) 
    {
        ParseError(line, "Missing second dot in range");
        return false;
    }

    if (!SimpleExpr(in, line, retVal2))
        return false;

    if (!retVal2.IsInt()) 
    {
        ParseError(line, "Run-Time Error-Non-integer index");
        return false;
    }

    if (retVal1.GetInt() > retVal2.GetInt()) 
    {
        ParseError(line, "Run-Time Error-Invalid range bounds");
        return false;
    }

    return true;
}
