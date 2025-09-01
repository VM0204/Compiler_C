// Header file for handling values and value types
val.h
#ifndef VALUE_H
#define VALUE_H

#include <iostream>
#include <string>
#include <queue>
#include <map>
#include <iomanip>
#include <stdexcept>
#include <cmath>
#include <sstream>

using namespace std;

// Value types
enum ValType { VINT, VREAL, VSTRING, VCHAR, VBOOL, VERR };

class Value 
{
    ValType T;
    bool    Btemp;
    int Itemp;
    double   Rtemp;
    string Stemp;
    char Ctemp;
    int strcurrLen;
    int strLen;
 
// Constructors
public:
    Value() : T(VERR), Btemp(false), Itemp(0), Rtemp(0.0), Stemp(""), Ctemp(0) {}
    Value(bool vb) : T(VBOOL), Btemp(vb), Itemp(0), Rtemp(0.0), Stemp(""), Ctemp(0) {}
    Value(int vi) : T(VINT), Btemp(false), Itemp(vi), Rtemp(0.0), Stemp(""), Ctemp(0) {}
    Value(double vr) : T(VREAL), Btemp(false), Itemp(0), Rtemp(vr), Stemp(""), Ctemp(0) {}
    Value(string vs) : T(VSTRING), Btemp(false), Itemp(0), Rtemp(0.0), Stemp(vs), Ctemp(0) 
{
    if(vs.length() == 0)
    {
    strcurrLen = 0;
    }
    else
    {
    strcurrLen = vs.length();
    strLen = strcurrLen;
    }
}
// Type query and getter methods
    Value(char vs) : T(VCHAR), Btemp(false), Itemp(0), Rtemp(0.0), Stemp(""), Ctemp(vs) {}
   
    ValType GetType() const { return T; }
    bool IsErr() const { return T == VERR; }
    bool IsString() const { return T == VSTRING; }
    bool IsReal() const {return T == VREAL;}
    bool IsBool() const {return T == VBOOL;}
    bool IsInt() const { return T == VINT; }
    bool IsChar() const {return T == VCHAR;}
   
    int GetInt() const { if( IsInt() ) return Itemp; throw "RUNTIME ERROR: Value not an Integer"; }
   
    string GetString() const { if( IsString() ) return Stemp; throw "RUNTIME ERROR: Value not a String"; }
   
    double GetReal() const { if( IsReal() ) return Rtemp; throw "RUNTIME ERROR: Value not an Float"; }
   
    bool GetBool() const {if(IsBool()) return Btemp; throw "RUNTIME ERROR: Value not a Boolean";}
   
    char GetChar() const {if(IsChar()) return Ctemp; throw "RUNTIME ERROR: Value not a Character";}
   
    void SetType(ValType type)
{
    T = type;
}

// Assign values with type checks
void SetInt(int val)
{
    if(IsInt())
    Itemp = val;
    else
    throw "RUNTIME ERROR: Value not an Integer";
}

void SetReal(double val)
{
    if(IsReal())
    Rtemp = val;
    else
    throw "RUNTIME ERROR: Value not an Float";   
}

void SetString(string val)
{
    if(IsString())  
    {
        if(val.length() <= strLen)
        {
            Stemp = val;
            strcurrLen = val.length();
        }
        else
        {
        Stemp = val.substr(0, strLen);
        }
    }   
    else
    throw "RUNTIME ERROR: Value not a String";
}

void SetBool(bool val)
{
    if(IsBool())
    Btemp = val;
    else
    throw "RUNTIME ERROR: Value not a Boolean";
}

void SetChar(char val)
{
    if(IsChar())
    Ctemp = val;
    else
    throw "RUNTIME ERROR: Value not a Character";
}

void SetstrLen(int len)
{
if(IsString())  
    strLen = len;
    else
    throw "RUNTIME ERROR: Value Type not a String";
}
// Overloaded operators
Value operator+(const Value& op) const;
   
Value operator-(const Value& op) const;
   
Value operator*(const Value& op) const;

Value operator/(const Value& op) const;
   
Value operator%(const Value & op) const;
         
Value operator==(const Value& op) const;
   
Value operator!=(const Value& op) const;
   
Value operator>(const Value& op) const;

Value operator<(const Value& op) const;

Value operator<=(const Value& op) const;

Value operator>=(const Value& op) const;

Value operator&&(const Value& op) const;
   
Value operator||(const Value& op) const;

Value operator!(void) const;

Value Concat(const Value & op) const;
   
Value Exp(const Value & op) const;

// Output stream operator
friend ostream& operator<<(ostream& out, const Value& op) 
{
    if( op.IsInt() ) out << op.Itemp;
    else if(op.IsBool()) out << (op.GetBool()? "true": "false");
    else if( op.IsChar() ) out << op.Ctemp ;
    else if( op.IsString() ) out << op.Stemp ;
    else if( op.IsReal()) out << fixed << showpoint << setprecision(2) << op.Rtemp;
    else if(op.IsErr()) out << "ERROR";
    return out;
}
};

inline Value Value::operator+(const Value& op) const 
{
    if (IsInt() && op.IsInt()) return Value(Itemp + op.Itemp);
    if (IsReal() && op.IsReal()) return Value(Rtemp + op.Rtemp);
    if (IsInt() && op.IsReal()) return Value(Itemp + op.Rtemp);
    if (IsReal() && op.IsInt()) return Value(Rtemp + op.Itemp);
    return Value();
}

inline Value Value::operator-(const Value& op) const 
{
    if (IsInt() && op.IsInt()) return Value(Itemp - op.Itemp);
    if (IsReal() && op.IsReal()) return Value(Rtemp - op.Rtemp);
    if (IsInt() && op.IsReal()) return Value(Itemp - op.Rtemp);
    if (IsReal() && op.IsInt()) return Value(Rtemp - op.Itemp);
    return Value();
}

inline Value Value::operator*(const Value& op) const 
{
    if (IsInt() && op.IsInt()) return Value(Itemp * op.Itemp);
    if (IsReal() && op.IsReal()) return Value(Rtemp * op.Rtemp);
    if (IsInt() && op.IsReal()) return Value(Itemp * op.Rtemp);
    if (IsReal() && op.IsInt()) return Value(Rtemp * op.Itemp);
    return Value();
}

inline Value Value::operator/(const Value& op) const 
{
    if ((op.IsInt() && op.Itemp == 0) || (op.IsReal() && op.Rtemp == 0.0)) return Value();
    if (IsInt() && op.IsInt()) return Value(Itemp / op.Itemp);
    if (IsReal() && op.IsReal()) return Value(Rtemp / op.Rtemp);
    if (IsInt() && op.IsReal()) return Value(Itemp / op.Rtemp);
    if (IsReal() && op.IsInt()) return Value(Rtemp / op.Itemp);
    return Value();
}

inline Value Value::operator%(const Value& op) const 
{
    if (IsInt() && op.IsInt() && op.Itemp != 0) return Value(Itemp % op.Itemp);
    return Value();
}

inline Value Value::operator==(const Value& op) const 
{
    if (T != op.T) return Value(false);
    switch (T) 
    {
        case VINT: return Value(Itemp == op.Itemp);
        case VREAL: return Value(Rtemp == op.Rtemp);
        case VSTRING: return Value(Stemp == op.Stemp);
        case VCHAR: return Value(Ctemp == op.Ctemp);
        case VBOOL: return Value(Btemp == op.Btemp);
        default: return Value(false);
    }
}

inline Value Value::operator!=(const Value& op) const 
{
    return Value(!((*this) == op).GetBool());
}

inline Value Value::operator<(const Value& op) const 
{
    if (IsInt() && op.IsInt()) return Value(Itemp < op.Itemp);
    if (IsReal() && op.IsReal()) return Value(Rtemp < op.Rtemp);
    if (IsInt() && op.IsReal()) return Value(Itemp < op.Rtemp);
    if (IsReal() && op.IsInt()) return Value(Rtemp < op.Itemp);
    return Value();
}

inline Value Value::operator<=(const Value& op) const 
{
    return Value((*this < op).GetBool() || (*this == op).GetBool());
}

inline Value Value::operator>(const Value& op) const 
{
    return Value(!(*this <= op).GetBool());
}

inline Value Value::operator>=(const Value& op) const 
{
    return Value(!(*this < op).GetBool());
}

inline Value Value::operator&&(const Value& op) const 
{
    if (IsBool() && op.IsBool()) return Value(Btemp && op.Btemp);
    return Value();
}

inline Value Value::operator||(const Value& op) const 
{
    if (IsBool() && op.IsBool()) return Value(Btemp || op.Btemp);
    return Value();
}

inline Value Value::operator!() const 
{
    if (IsBool()) return Value(!Btemp);
    return Value();
}

inline Value Value::Concat(const Value& op) const 
{
    if (IsString() && op.IsString()) return Value(Stemp + op.Stemp);
    return Value();
}

inline Value Value::Exp(const Value& op) const 
{
    if (IsInt() && op.IsInt()) return Value(pow(Itemp, op.Itemp));
    if (IsReal() && op.IsReal()) return Value(pow(Rtemp, op.Rtemp));
    if (IsInt() && op.IsReal()) return Value(pow(Itemp, op.Rtemp));
    if (IsReal() && op.IsInt()) return Value(pow(Rtemp, op.Itemp));
    return Value();
}
#endif