/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */

/*
 *  Main authors:
 *     Guido Tack <guido.tack@monash.edu>
 */

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __MINIZINC_TYPE_HH__
#define __MINIZINC_TYPE_HH__

#include <string>
#include <sstream>

namespace MiniZinc {

  /// Type of a MiniZinc expression
  class Type {
  public:
    /// Type-inst
    enum TypeInst { TI_PAR, TI_VAR };
    /// Basic type
    enum BaseType { BT_TOP, BT_BOOL, BT_INT, BT_FLOAT, BT_STRING, BT_ANN,
                    BT_BOT, BT_UNKNOWN };
    /// Whether the expression is plain or set
    enum SetType { ST_PLAIN, ST_SET };
    /// Whether the expression is normal or optional
    enum OptType { OT_PRESENT, OT_OPTIONAL };
    /// Whether the par expression contains a var argument
    enum ContainsVarType { CV_NO, CV_YES };
  private:
    unsigned int _ti : 1;
    unsigned int _bt : 4;
    unsigned int _st  : 1;
    unsigned int _ot  : 1;
    unsigned int _cv  : 1;
    /// Number of array dimensions
    int _dim : 19;
  public:
    /// Default constructor
    Type(void) : _ti(TI_PAR), _bt(BT_UNKNOWN), _st(ST_PLAIN),
                 _ot(OT_PRESENT), _cv(CV_NO), _dim(0) {}
    
    /// Access type-inst
    TypeInst ti(void) const { return static_cast<TypeInst>(_ti); }
    /// Set type-inst
    void ti(const TypeInst& t) {
      _ti = t;
      if (t==TI_VAR)
        _cv=CV_YES;
    }

    /// Access basic type
    BaseType bt(void) const { return static_cast<BaseType>(_bt); }
    /// Set basic type
    void bt(const BaseType& b) { _bt = b; }
    
    /// Access set type
    SetType st(void) const { return static_cast<SetType>(_st); }
    /// Set set type
    void st(const SetType& s) { _st = s; }
    
    /// Access opt type
    OptType ot(void) const { return static_cast<OptType>(_ot); }
    /// Set opt type
    void ot(const OptType& o) { _ot = o; }
    
    /// Access var-in-par type
    bool cv(void) const { return static_cast<ContainsVarType>(_cv) == CV_YES; }
    /// Set var-in-par type
    void cv(bool b) { _cv = b ? CV_YES : CV_NO; }
    
    /// Access dimensions
    int dim(void) const { return _dim; }
    /// Set dimensions
    void dim(int d) { _dim = d; }

  protected:
    /// Constructor
    Type(const TypeInst& ti, const BaseType& bt, const SetType& st,
         int dim)
      : _ti(ti), _bt(bt), _st(st), _ot(OT_PRESENT), _cv(ti==TI_VAR ? CV_YES : CV_NO), _dim(dim) {}
  public:
    static Type parint(int dim=0) {
      return Type(TI_PAR,BT_INT,ST_PLAIN,dim);
    }
    static Type parbool(int dim=0) {
      return Type(TI_PAR,BT_BOOL,ST_PLAIN,dim);
    }
    static Type parfloat(int dim=0) {
      return Type(TI_PAR,BT_FLOAT,ST_PLAIN,dim);
    }
    static Type parstring(int dim=0) {
      return Type(TI_PAR,BT_STRING,ST_PLAIN,dim);
    }
    static Type ann(int dim=0) {
      return Type(TI_PAR,BT_ANN,ST_PLAIN,dim);
    }
    static Type parsetint(int dim=0) {
      return Type(TI_PAR,BT_INT,ST_SET,dim);
    }
    static Type parsetbool(int dim=0) {
      return Type(TI_PAR,BT_BOOL,ST_SET,dim);
    }
    static Type parsetfloat(int dim=0) {
      return Type(TI_PAR,BT_FLOAT,ST_SET,dim);
    }
    static Type parsetstring(int dim=0) {
      return Type(TI_PAR,BT_STRING,ST_SET,dim);
    }
    static Type varint(int dim=0) {
      return Type(TI_VAR,BT_INT,ST_PLAIN,dim);
    }
    static Type varbool(int dim=0) {
      return Type(TI_VAR,BT_BOOL,ST_PLAIN,dim);
    }
    static Type varfloat(int dim=0) {
      return Type(TI_VAR,BT_FLOAT,ST_PLAIN,dim);
    }
    static Type varsetint(int dim=0) {
      return Type(TI_VAR,BT_INT,ST_SET,dim);
    }
    static Type varbot(int dim=0) {
      return Type(TI_VAR,BT_BOT,ST_PLAIN,dim);
    }
    static Type bot(int dim=0) {
      return Type(TI_PAR,BT_BOT,ST_PLAIN,dim);
    }
    static Type top(int dim=0) {
      return Type(TI_PAR,BT_TOP,ST_PLAIN,dim);
    }
    static Type vartop(int dim=0) {
      return Type(TI_VAR,BT_TOP,ST_PLAIN,dim);
    }
    static Type optvartop(int dim=0) {
      Type t(TI_VAR,BT_TOP,ST_PLAIN,dim);
      t._ot = OT_OPTIONAL;
      return t;
    }

    bool isunknown(void) const { return _bt==BT_UNKNOWN; }
    bool isplain(void) const {
      return _dim==0 && _st==ST_PLAIN && _ot==OT_PRESENT;
    }
    bool isint(void) const { return _dim==0 && _st==ST_PLAIN && _bt==BT_INT; }
    bool isbot(void) const { return _bt==BT_BOT; }
    bool isfloat(void) const { return _dim==0 && _st==ST_PLAIN && _bt==BT_FLOAT; }
    bool isbool(void) const { return _dim==0 && _st==ST_PLAIN && _bt==BT_BOOL; }
    bool isstring(void) const { return isplain() && _bt==BT_STRING; }
    bool isvar(void) const { return _ti!=TI_PAR; }
    bool isvarbool(void) const { return _ti==TI_VAR && _dim==0 && _st==ST_PLAIN && _bt==BT_BOOL && _ot==OT_PRESENT; }
    bool isvarfloat(void) const { return _ti==TI_VAR && _dim==0 && _st==ST_PLAIN && _bt==BT_FLOAT && _ot==OT_PRESENT; }
    bool isvarint(void) const { return _ti==TI_VAR && _dim==0 && _st==ST_PLAIN && _bt==BT_INT && _ot==OT_PRESENT; }
    bool ispar(void) const { return _ti==TI_PAR; }
    bool isopt(void) const { return _ot==OT_OPTIONAL; }
    bool ispresent(void) const { return _ot==OT_PRESENT; }
    bool is_set(void) const { return _dim==0 && _st==ST_SET; }
    bool isintset(void) const {
      return is_set() && (_bt==BT_INT || _bt==BT_BOT);
    }
    bool isboolset(void) const {
      return is_set() && (_bt==BT_BOOL || _bt==BT_BOT);
    }
    bool isann(void) const { return isplain() && _bt==BT_ANN; }
    bool isintarray(void) const {
      return _dim==1 && _st==ST_PLAIN && _ot==OT_PRESENT && _bt==BT_INT;
    }
    bool isfloatarray(void) const {
      return _dim==1 && _st==ST_PLAIN && _ot==OT_PRESENT && _bt==BT_FLOAT;
    }
    bool isboolarray(void) const {
      return _dim==1 && _st==ST_PLAIN && _ot==OT_PRESENT && _bt==BT_BOOL;
    }
    bool isintsetarray(void) const {
      return _dim==1 && _st==ST_SET && _bt==BT_INT;
    }

    bool operator== (const Type& t) const {
      return _ti==t._ti && _bt==t._bt && _st==t._st &&
             _ot==t._ot && _dim==t._dim;
    }
    bool operator!= (const Type& t) const {
      return !this->operator==(t);
    }
  // protected:

    int toInt(void) const {
      return
      + ((1-static_cast<int>(_st))<<28)
      + (static_cast<int>(_bt)<<24)
      + (static_cast<int>(_ti)<<21)
      + (static_cast<int>(_ot)<<20)
      + (_dim == -1 ? 1 : (_dim == 0 ? 0 : _dim+1));
    }
    static Type fromInt(int i) {
      Type t;
      t._st = 1-static_cast<SetType>((i >> 28) & 0x1);
      t._bt = static_cast<BaseType>((i >> 24) & 0xF);
      t._ti = static_cast<TypeInst>((i >> 21) & 0x7);
      t._ot = static_cast<OptType>((i >> 20) & 0x1);
      int dim = (i & 0xFFFFF);
      t._dim =  (dim == 0 ? 0 : (dim==1 ? -1 : dim-1));
      return t;
    }
    std::string toString(void) const {
      std::ostringstream oss;
      if (_dim>0) {
        oss<<"array[int";
        for (int i=1; i<_dim; i++)
          oss << ",int";
        oss<<"] of ";
      }
      if (_dim<0)
        oss<<"array[$_] of ";
      switch (_ti) {
        case TI_PAR: break;
        case TI_VAR: oss<<"var "; break;
      }
      if (_ot==OT_OPTIONAL) oss<<"opt ";
      if (_st==ST_SET) oss<<"set of ";
      switch (_bt) {
        case BT_INT: oss<<"int"; break;
        case BT_BOOL: oss<<"bool"; break;
        case BT_FLOAT: oss<<"float"; break;
        case BT_STRING: oss<<"string"; break;
        case BT_ANN: oss<<"ann"; break;
        case BT_BOT: oss<<"bot"; break;
        case BT_TOP: oss<<"top"; break;
        case BT_UNKNOWN: oss<<"??? "; break;
      }
      return oss.str();
    }
  public:
    /// Check if \a bt0 is a subtype of \a bt1
    static bool bt_subtype(const BaseType& bt0, const BaseType& bt1) {
      if (bt0==bt1)
        return true;
      switch (bt0) {
        case BT_BOOL: return (bt1==BT_INT || bt1==BT_FLOAT);
        case BT_INT: return bt1==BT_FLOAT;
        default: return false;
      }
    }

    /// Check if this type is a subtype of \a t
    bool isSubtypeOf(const Type& t) const {
      if (_dim==0 && t._dim!=0 && _st==ST_SET && t._st==ST_PLAIN &&
          ( bt()==BT_BOT || bt_subtype(bt(), t.bt()) || t.bt()==BT_TOP) && _ti==TI_PAR &&
          (_ot==OT_PRESENT || _ot==t._ot) )
        return true;
      // either same dimension or t has variable dimension
      if (_dim!=t._dim && (_dim==0 || t._dim!=-1))
        return false;
      // same type, this is present or both optional
      if (_ti==t._ti && bt_subtype(bt(),t.bt()) && _st==t._st)
        return _ot==OT_PRESENT || _ot==t._ot;
      // this is par other than that same type as t
      if (_ti==TI_PAR && bt_subtype(bt(),t.bt()) && _st==t._st)
        return _ot==OT_PRESENT || _ot==t._ot;
      if ( _ti==TI_PAR && t._bt==BT_BOT)
        return true;
      if ((_ti==t._ti || _ti==TI_PAR) && _bt==BT_BOT && (_st==t._st || _st==ST_PLAIN))
        return _ot==OT_PRESENT || _ot==t._ot;
      if (t._bt==BT_TOP && (_ot==OT_PRESENT || _ot==t._ot) &&
          (t._st==ST_PLAIN || _st==t._st) &&
          (_ti==TI_PAR || t._ti==TI_VAR))
        return true;
      return false;
    }

    /// Compare types
    int cmp(const Type& t) const {
      return toInt()<t.toInt() ? -1 : (toInt()>t.toInt() ? 1 : 0);
    }

  };
  
};

#endif
