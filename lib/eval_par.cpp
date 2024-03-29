/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */

/*
 *  Main authors:
 *     Guido Tack <guido.tack@monash.edu>
 */

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <minizinc/eval_par.hh>
#include <minizinc/astexception.hh>
#include <minizinc/iter.hh>
#include <minizinc/hash.hh>
#include <minizinc/copy.hh>
#include <minizinc/astiterator.hh>
#include <minizinc/flatten.hh>

namespace MiniZinc {

  template<class E>
  typename E::Val eval_id(EnvI& env, Expression* e, bool eval_outputmodel) {
    Id* id = e->cast<Id>();
    if (id->decl() == NULL)
      throw EvalError(env, e->loc(), "undeclared identifier", id->str().str());
    VarDecl* vd = id->decl();
    if(vd->type().ispar() && vd->e()) {
      
    }
    //if(!eval_outputmodel) {
    else {
      while (vd->flat() && vd->flat() != vd) 
        vd = vd->flat();   
    }
    if (vd->e() == NULL)
      throw EvalError(env, vd->loc(), "cannot evaluate expression", id->str().str());
    typename E::Val r = E::e(env,vd->e());
    if (!vd->evaluated() && (vd->toplevel() || vd->type().dim() > 0) ) {
      Expression* ne = E::exp(r);
      vd->e(ne);
      vd->evaluated(true);
    }
    return r;
  }

  class EvalIntLit {
  public:
    typedef IntLit* Val;
    typedef Expression* ArrayVal;
    static IntLit* e(EnvI& env, Expression* e, bool om = false) {
      return IntLit::a(eval_int(env, e, om));
    }
    static Expression* exp(IntLit* e) { return e; }
  };
  class EvalIntVal {
  public:
    typedef IntVal Val;
    typedef IntVal ArrayVal;
    static IntVal e(EnvI& env, Expression* e) {
      return eval_int(env, e);
    }
    static Expression* exp(IntVal e) { return IntLit::a(e); }
  };
  class EvalFloatVal {
  public:
    typedef FloatVal Val;
    static FloatVal e(EnvI& env, Expression* e) {
      return eval_float(env, e);
    }
    static Expression* exp(FloatVal e) { return FloatLit::a(e); }
  };
  class EvalFloatLit {
  public:
    typedef FloatLit* Val;
    typedef Expression* ArrayVal;
    static FloatLit* e(EnvI& env, Expression* e, bool om = false) {
      return FloatLit::a(eval_float(env, e, om));
    }
    static Expression* exp(Expression* e) { return e; }
  };
  class EvalString {
  public:
    typedef std::string Val;
    typedef std::string ArrayVal;
    static std::string e(EnvI& env, Expression* e) {
      return eval_string(env, e);
    }
    static Expression* exp(const std::string& e) { return new StringLit(Location(),e); }
  };
  class EvalStringLit {
  public:
    typedef StringLit* Val;
    typedef Expression* ArrayVal;
    static StringLit* e(EnvI& env, Expression* e, bool om = false) {
      return new StringLit(Location(),eval_string(env, e, om));
    }
    static Expression* exp(Expression* e) { return e; }
  };
  class EvalBoolLit {
  public:
    typedef BoolLit* Val;
    typedef Expression* ArrayVal;
    static BoolLit* e(EnvI& env, Expression* e, bool om = false) {
      return constants().boollit(eval_bool(env, e, om));
    }
    static Expression* exp(Expression* e) { return e; }
  };
  class EvalBoolVal {
  public:
    typedef bool Val;
    static bool e(EnvI& env, Expression* e) {
      return eval_bool(env, e);
    }
    static Expression* exp(bool e) { return constants().boollit(e); }
  };
  class EvalArrayLit {
  public:
    typedef ArrayLit* Val;
    typedef Expression* ArrayVal;
    static ArrayLit* e(EnvI& env, Expression* e, bool om = false) {
      return eval_array_lit(env, e, om);
    }
    static Expression* exp(Expression* e) { return e; }
  };
  class EvalArrayLitCopy {
  public:
    typedef ArrayLit* Val;
    typedef Expression* ArrayVal;
    static ArrayLit* e(EnvI& env, Expression* e) {
      return copy(env,eval_array_lit(env, e),true)->cast<ArrayLit>();
    }
    static Expression* exp(Expression* e) { return e; }
  };
  class EvalIntSet {
  public:
    typedef IntSetVal* Val;
    static IntSetVal* e(EnvI& env, Expression* e) {
      return eval_intset(env, e);
    }
    static Expression* exp(IntSetVal* e) { return new SetLit(Location(),e); }
  };
  class EvalBoolSet {
  public:
    typedef IntSetVal* Val;
    static IntSetVal* e(EnvI& env, Expression* e) {
      return eval_boolset(env, e);
    }
    static Expression* exp(IntSetVal* e) { return new SetLit(Location(),e); }
  };
  class EvalSetLit {
  public:
    typedef SetLit* Val;
    typedef Expression* ArrayVal;
    static SetLit* e(EnvI& env, Expression* e, bool om = false) {
      return new SetLit(e->loc(),eval_intset(env, e, om));
    }
    static Expression* exp(Expression* e) { return e; }
  };
  class EvalBoolSetLit {
  public:
    typedef SetLit* Val;
    typedef Expression* ArrayVal;
    static SetLit* e(EnvI& env, Expression* e, bool om = false) {
      return new SetLit(e->loc(),eval_boolset(env, e, om));
    }
    static Expression* exp(Expression* e) { return e; }
  };
  class EvalNone {
  public:
    typedef Expression* Val;
    typedef Expression* ArrayVal;
    static Expression* e(EnvI&, Expression* e) {
      return e;
    }
    static Expression* exp(Expression* e) { return e; }
  };
  class EvalCopy {
  public:
    typedef Expression* Val;
    typedef Expression* ArrayVal;
    static Expression* e(EnvI& env, Expression* e) {
      return copy(env,e,true);
    }
    static Expression* exp(Expression* e) { return e; }
  };
  class EvalPar {
  public:
    typedef Expression* Val;
    typedef Expression* ArrayVal;
    static Expression* e(EnvI& env, Expression* e) {
      return eval_par(env, e);
    }
    static Expression* exp(Expression* e) { return e; }
  };

  void checkDom(EnvI& env, Id* arg, IntSetVal* dom, Expression* e) {
    bool oob = false;
    if (e->type().isintset()) {
      IntSetVal* ev = eval_intset(env, e);
      IntSetRanges ev_r(ev);
      IntSetRanges dom_r(dom);
      oob = !Ranges::subset(ev_r, dom_r);
    } else {
      oob = !dom->contains(eval_int(env,e));
    }
    if (oob) {
      std::ostringstream oss;
      oss << "value for argument `" << *arg << "' out of bounds";
      throw EvalError(env, e->loc(), oss.str());
    }
  }

  void checkDom(EnvI& env, Id* arg, FloatVal dom_min, FloatVal dom_max, Expression* e) {
    FloatVal ev = eval_float(env, e);
    if (ev < dom_min || ev > dom_max) {
      std::ostringstream oss;
      oss << "value for argument `" << *arg << "' out of bounds";
      throw EvalError(env, e->loc(), oss.str());
    }
  }
  
  template<class Eval>
  typename Eval::Val eval_call(EnvI& env, Call* ce, bool om) {
    std::vector<Expression*> previousParameters(ce->decl()->params().size());
    for (unsigned int i=ce->decl()->params().size(); i--;) {
      VarDecl* vd = ce->decl()->params()[i];
      previousParameters[i] = vd->e();
      vd->flat(vd);
      vd->e(eval_par(env, ce->args()[i],om));
      if (vd->e()->type().ispar()) {
        if (Expression* dom = vd->ti()->domain()) {
          if (!dom->isa<TIId>()) {
            if (vd->e()->type().bt()==Type::BT_INT) {
              IntSetVal* isv = eval_intset(env, dom,om);
              if (vd->e()->type().dim() > 0) {
                ArrayLit* al = eval_array_lit(env, vd->e(),om);
                for (unsigned int i=0; i<al->v().size(); i++) {
                  checkDom(env, vd->id(), isv, al->v()[i]);
                }
              } else {
                checkDom(env, vd->id(),isv, vd->e());
              }
            } else if (vd->e()->type().bt()==Type::BT_FLOAT) {
              GCLock lock;
              BinOp* bo = dom->cast<BinOp>();
              FloatVal dom_min = eval_float(env,bo->lhs(),om);
              FloatVal dom_max = eval_float(env,bo->rhs(),om);
              checkDom(env, vd->id(), dom_min, dom_max, vd->e());
            }
          }
        }
      }
    }
    typename Eval::Val ret = Eval::e(env,ce->decl()->e());
    for (unsigned int i=ce->decl()->params().size(); i--;) {
      VarDecl* vd = ce->decl()->params()[i];
      vd->e(previousParameters[i]);
      vd->flat(vd->e() ? vd : NULL);
    }
    return ret;
  }
  
  ArrayLit* eval_array_comp(EnvI& env, Comprehension* e, bool om = false) {
    ArrayLit* ret;
    if (e->type() == Type::parint(1)) {
      std::vector<Expression*> a = eval_comp<EvalIntLit>(env,e, om);
      ret = new ArrayLit(e->loc(),a);
    } else if (e->type() == Type::parbool(1)) {
      std::vector<Expression*> a = eval_comp<EvalBoolLit>(env,e,om);
      ret = new ArrayLit(e->loc(),a);
    } else if (e->type() == Type::parfloat(1)) {
      std::vector<Expression*> a = eval_comp<EvalFloatLit>(env,e,om);
      ret = new ArrayLit(e->loc(),a);
    } else if (e->type() == Type::parsetint(1)) {
      std::vector<Expression*> a = eval_comp<EvalSetLit>(env,e,om);
      ret = new ArrayLit(e->loc(),a);
    } else if (e->type() == Type::parstring(1)) {
      std::vector<Expression*> a = eval_comp<EvalStringLit>(env,e,om);
      ret = new ArrayLit(e->loc(),a);
    } else {
      std::vector<Expression*> a = eval_comp<EvalCopy>(env,e,om);
      ret = new ArrayLit(e->loc(),a);
    }
    ret->type(e->type());
    return ret;
  }
  
  ArrayLit* eval_array_lit(EnvI& env, Expression* e, bool om) {
    switch (e->eid()) {
    case Expression::E_INTLIT:
    case Expression::E_FLOATLIT:
    case Expression::E_BOOLLIT:
    case Expression::E_STRINGLIT:
    case Expression::E_SETLIT:
    case Expression::E_ANON:
    case Expression::E_TI:
    case Expression::E_TIID:
    case Expression::E_VARDECL:
    case Expression::E_MODEL:
      throw EvalError(env, e->loc(), "not an array expression");
    case Expression::E_ID:
      return eval_id<EvalArrayLit>(env,e, om);
    case Expression::E_ARRAYLIT:
    {
      return e->cast<ArrayLit>();
    }
    case Expression::E_ARRAYACCESS:
      throw EvalError(env, e->loc(),"arrays of arrays not supported");
    case Expression::E_COMP:
      return eval_array_comp(env,e->cast<Comprehension>(), om);
    case Expression::E_ITE:
      {
        ITE* ite = e->cast<ITE>();
        for (int i=0; i<ite->size(); i++) {
          if (eval_bool(env,ite->e_if(i)))
            return eval_array_lit(env,ite->e_then(i), om);
        }
        return eval_array_lit(env,ite->e_else(), om);
      }
    case Expression::E_BINOP:
      {
        BinOp* bo = e->cast<BinOp>();
        if (bo->op()==BOT_PLUSPLUS) {
          ArrayLit* al0 = eval_array_lit(env,bo->lhs(), om);
          ArrayLit* al1 = eval_array_lit(env,bo->rhs(), om);
          std::vector<Expression*> v(al0->v().size()+al1->v().size());
          for (unsigned int i=al0->v().size(); i--;)
            v[i] = al0->v()[i];
          for (unsigned int i=al1->v().size(); i--;)
            v[al0->v().size()+i] = al1->v()[i];
          ArrayLit* ret = new ArrayLit(e->loc(),v);
          ret->flat(al0->flat() && al1->flat());
          ret->type(e->type());
          return ret;
        } else {
          throw EvalError(env, e->loc(), "not an array expression", bo->opToString());
        }
      }
      break;
    case Expression::E_UNOP:
      throw EvalError(env, e->loc(), "not an array expression");
    case Expression::E_CALL:
      {
        Call* ce = e->cast<Call>();
        if (ce->decl()==NULL)
          throw EvalError(env, e->loc(), "undeclared function", ce->id());
        
        if (ce->decl()->_builtins.e)
          return eval_array_lit(env,ce->decl()->_builtins.e(env,ce), om);

        if (ce->decl()->e()==NULL)
          throw EvalError(env, ce->loc(), "internal error: missing builtin '"+ce->id().str()+"'");

        return eval_call<EvalArrayLitCopy>(env,ce,om);
      }
    case Expression::E_LET:
      {
        Let* l = e->cast<Let>();
        l->pushbindings();
        ArrayLit* l_in = eval_array_lit(env,l->in(), om);
        ArrayLit* ret = copy(env,l_in,true)->cast<ArrayLit>();
        ret->flat(l_in->flat());
        l->popbindings();
        return ret;
      }
    }
    assert(false); return NULL;
  }

  Expression* eval_arrayaccess(EnvI& env, ArrayLit* al, const std::vector<IntVal>& dims,
                               bool& success, bool om) {
    success = true;
    assert(al->dims() == dims.size());
    IntVal realidx = 0;
    int realdim = 1;
    for (int i=0; i<al->dims(); i++)
      realdim *= al->max(i)-al->min(i)+1;
    for (int i=0; i<al->dims(); i++) {
      IntVal ix = dims[i];
      if (ix < al->min(i) || ix > al->max(i)) {
        success = false;
        Type t = al->type();
        t.dim(0);
        if (t.isint())
          return IntLit::a(0);
        if (t.isbool())
          return constants().lit_false;
        if (t.isfloat())
          return FloatLit::a(0.0);
        if (t.st() == Type::ST_SET || t.isbot()) {
          SetLit* ret = new SetLit(Location(),std::vector<Expression*>());
          ret->type(t);
          return ret;
        }
        if (t.isstring())
          return new StringLit(Location(),"");
        throw EvalError(env, al->loc(), "Internal error: unexpected type in array access expression");
      }
      realdim /= al->max(i)-al->min(i)+1;
      realidx += (ix-al->min(i))*realdim;
    }    
    assert(realidx >= 0 && realidx <= al->v().size());
    return al->v()[static_cast<unsigned int>(realidx.toInt())];
  }
  Expression* eval_arrayaccess(EnvI& env, ArrayAccess* e, bool& success, bool om) {
    ArrayLit* al = eval_array_lit(env,e->v(),om);
    std::vector<IntVal> dims(e->idx().size());
    for (unsigned int i=e->idx().size(); i--;) {
      dims[i] = eval_int(env,e->idx()[i],om);
    }    
    return eval_arrayaccess(env,al,dims,success,om);
  }
  Expression* eval_arrayaccess(EnvI& env, ArrayAccess* e, bool om = false) {
    bool success;
    Expression* ret = eval_arrayaccess(env,e,success,om);
    if (success)
      return ret;
    else
      throw EvalError(env, e->loc(), "array access out of bounds");
  }
  
  IntSetVal* eval_intset(EnvI& env, Expression* e, bool om) {
    if (SetLit* sl = e->dyn_cast<SetLit>()) {
      if (sl->isv())
        return sl->isv();
    }
    CallStackItem csi(env,e);
    switch (e->eid()) {
    case Expression::E_SETLIT:
      {
        SetLit* sl = e->cast<SetLit>();
        std::vector<IntVal> vals(sl->v().size());
        for (unsigned int i=0; i<sl->v().size(); i++)
          vals[i] = eval_int(env,sl->v()[i],om);
        return IntSetVal::a(vals);
      }
    case Expression::E_BOOLLIT:
    case Expression::E_INTLIT: 
    case Expression::E_FLOATLIT:
    case Expression::E_STRINGLIT:
    case Expression::E_ANON:
    case Expression::E_TIID:
    case Expression::E_VARDECL:
    case Expression::E_TI:
    case Expression::E_UNOP:
      throw EvalError(env, e->loc(),"not a set of int expression");
      break;
    case Expression::E_ARRAYLIT:
      {
        ArrayLit* al = e->cast<ArrayLit>();
        std::vector<IntVal> vals(al->v().size());
        for (unsigned int i=0; i<al->v().size(); i++)
          vals[i] = eval_int(env,al->v()[i],om);
        return IntSetVal::a(vals);
      }
      break;
    case Expression::E_COMP:
      {
        Comprehension* c = e->cast<Comprehension>();
        std::vector<IntVal> a = eval_comp<EvalIntVal>(env,c,om);
        return IntSetVal::a(a);
      }
    case Expression::E_ID:
      {
        GCLock lock;
        return eval_id<EvalSetLit>(env,e,om)->isv();
      }
      break;
    case Expression::E_ARRAYACCESS:
      {
        GCLock lock;
        return eval_intset(env,eval_arrayaccess(env,e->cast<ArrayAccess>(),om),om);
      }
      break;
    case Expression::E_ITE:
      {
        ITE* ite = e->cast<ITE>();
        for (int i=0; i<ite->size(); i++) {
          if (eval_bool(env,ite->e_if(i),om))
            return eval_intset(env,ite->e_then(i),om);
        }
        return eval_intset(env,ite->e_else(),om);
      }
      break;
    case Expression::E_BINOP:
      {
        BinOp* bo = e->cast<BinOp>();
        Expression* lhs = eval_par(env, bo->lhs(),om);
        Expression* rhs = eval_par(env, bo->rhs(),om);
        if (lhs->type().isintset() && rhs->type().isintset()) {
          IntSetVal* v0 = eval_intset(env,lhs,om);
          IntSetVal* v1 = eval_intset(env,rhs,om);
          IntSetRanges ir0(v0);
          IntSetRanges ir1(v1);
          switch (bo->op()) {
          case BOT_UNION:
            {
              Ranges::Union<IntSetRanges,IntSetRanges> u(ir0,ir1);
              return IntSetVal::ai(u);
            }
          case BOT_DIFF:
            {
              Ranges::Diff<IntSetRanges,IntSetRanges> u(ir0,ir1);
              return IntSetVal::ai(u);
            }
          case BOT_SYMDIFF:
            {
              Ranges::Union<IntSetRanges,IntSetRanges> u(ir0,ir1);
              Ranges::Inter<IntSetRanges,IntSetRanges> i(ir0,ir1);
              Ranges::Diff<Ranges::Union<IntSetRanges,IntSetRanges>,
                           Ranges::Inter<IntSetRanges,IntSetRanges>> sd(u,i);
              return IntSetVal::ai(sd);
            }
          case BOT_INTERSECT:
            {
              Ranges::Inter<IntSetRanges,IntSetRanges> u(ir0,ir1);
              return IntSetVal::ai(u);
            }
          default: throw EvalError(env, e->loc(),"not a set of int expression", bo->opToString());
          }
        } else if (lhs->type().isint() && rhs->type().isint()) {
          if (bo->op() != BOT_DOTDOT)
            throw EvalError(env, e->loc(), "not a set of int expression", bo->opToString());
          return IntSetVal::a(eval_int(env,lhs,om),
                              eval_int(env,rhs,om));
        } else {
          throw EvalError(env, e->loc(), "not a set of int expression", bo->opToString());
        }
      }
      break;
    case Expression::E_CALL:
      {
        Call* ce = e->cast<Call>();
        if (ce->decl()==NULL)
          throw EvalError(env, e->loc(), "undeclared function", ce->id());
        
        if (ce->decl()->_builtins.s)
          return ce->decl()->_builtins.s(env,ce);

        if (ce->decl()->_builtins.e)
          return eval_intset(env,ce->decl()->_builtins.e(env,ce),om);

        if (ce->decl()->e()==NULL)
          throw EvalError(env, ce->loc(), "internal error: missing builtin '"+ce->id().str()+"'");
        
        return eval_call<EvalIntSet>(env,ce,om);
      }
      break;
    case Expression::E_LET:
      {
        Let* l = e->cast<Let>();
        l->pushbindings();
        IntSetVal* ret = eval_intset(env,l->in(),om);
        l->popbindings();
        return ret;
      }
      break;
    default: assert(false); return NULL;
    }
  }

  bool eval_bool(EnvI& env, Expression* e, bool om) {
    if (BoolLit* bl = e->dyn_cast<BoolLit>()) {
      return bl->v();
    }
    CallStackItem csi(env,e);
    switch (e->eid()) {
    case Expression::E_INTLIT:
    case Expression::E_FLOATLIT:
    case Expression::E_STRINGLIT:
    case Expression::E_ANON:
    case Expression::E_TIID:
    case Expression::E_SETLIT:
    case Expression::E_ARRAYLIT:
    case Expression::E_COMP:
    case Expression::E_VARDECL:
    case Expression::E_TI:
      assert(false);
      throw EvalError(env, e->loc(),"not a bool expression");
      break;
    case Expression::E_ID:
      {
        GCLock lock;
        return eval_id<EvalBoolLit>(env,e,om)->v();
      }
      break;
    case Expression::E_ARRAYACCESS:
      {
        GCLock lock;
        return eval_bool(env,eval_arrayaccess(env,e->cast<ArrayAccess>(),om),om);
      }
      break;
    case Expression::E_ITE:
      {
        ITE* ite = e->cast<ITE>();
        for (int i=0; i<ite->size(); i++) {
          if (eval_bool(env,ite->e_if(i),om))
            return eval_bool(env,ite->e_then(i),om);
        }
        return eval_bool(env,ite->e_else(),om);
      }
      break;
    case Expression::E_BINOP:
      {
        BinOp* bo = e->cast<BinOp>();
        Expression* lhs = eval_par(env, bo->lhs(),om);
        Expression* rhs = eval_par(env, bo->rhs(),om);
        if ( bo->op()==BOT_EQ && (lhs->type().isopt() || rhs->type().isopt()) ) {
          if (lhs == constants().absent || rhs==constants().absent)
            return lhs==rhs;
        }
        if (lhs->type().isbool() && rhs->type().isbool()) {
          switch (bo->op()) {
          case BOT_LE: return eval_bool(env,lhs,om)<eval_bool(env,rhs,om);
          case BOT_LQ: return eval_bool(env,lhs,om)<=eval_bool(env,rhs,om);
          case BOT_GR: return eval_bool(env,lhs,om)>eval_bool(env,rhs,om);
          case BOT_GQ: return eval_bool(env,lhs,om)>=eval_bool(env,rhs,om);
          case BOT_EQ: return eval_bool(env,lhs,om)==eval_bool(env,rhs,om);
          case BOT_NQ: return eval_bool(env,lhs,om)!=eval_bool(env,rhs,om);
          case BOT_EQUIV: return eval_bool(env,lhs,om)==eval_bool(env,rhs,om);
          case BOT_IMPL: return (!eval_bool(env,lhs,om))||eval_bool(env,rhs,om);
          case BOT_RIMPL: return (!eval_bool(env,rhs,om))||eval_bool(env,lhs,om);
          case BOT_OR: return eval_bool(env,lhs,om)||eval_bool(env,rhs,om);
          case BOT_AND: return eval_bool(env,lhs,om)&&eval_bool(env,rhs,om);
          case BOT_XOR: return eval_bool(env,lhs,om)^eval_bool(env,rhs,om);
          default:
            assert(false);
            throw EvalError(env, e->loc(),"not a bool expression", bo->opToString());
          }
        } else if (lhs->type().isint() && rhs->type().isint()) {
          IntVal v0 = eval_int(env,lhs,om);
          IntVal v1 = eval_int(env,rhs,om);
          switch (bo->op()) {
          case BOT_LE: return v0<v1;
          case BOT_LQ: return v0<=v1;
          case BOT_GR: return v0>v1;
          case BOT_GQ: return v0>=v1;
          case BOT_EQ: return v0==v1;
          case BOT_NQ: return v0!=v1;
          default:
            assert(false);
            throw EvalError(env, e->loc(),"not a bool expression", bo->opToString());
          }
        } else if (lhs->type().isfloat() && rhs->type().isfloat()) {
          FloatVal v0 = eval_float(env,lhs,om);
          FloatVal v1 = eval_float(env,rhs,om);
          switch (bo->op()) {
          case BOT_LE: return v0<v1;
          case BOT_LQ: return v0<=v1;
          case BOT_GR: return v0>v1;
          case BOT_GQ: return v0>=v1;
          case BOT_EQ: return v0==v1;
          case BOT_NQ: return v0!=v1;
          default:
            assert(false);
            throw EvalError(env, e->loc(),"not a bool expression", bo->opToString());
          }
        } else if (lhs->type().isint() && rhs->type().isintset()) {
          IntVal v0 = eval_int(env,lhs,om); 
          GCLock lock;
          IntSetVal* v1 = eval_intset(env,rhs,om);
          switch (bo->op()) {
          case BOT_IN: return v1->contains(v0);
          default:
            assert(false);
            throw EvalError(env, e->loc(),"not a bool expression", bo->opToString());
          }
        } else if (lhs->type().is_set() && rhs->type().is_set()) {
          GCLock lock;
          IntSetVal* v0 = eval_intset(env,lhs,om);
          IntSetVal* v1 = eval_intset(env,rhs,om);
          IntSetRanges ir0(v0);
          IntSetRanges ir1(v1);
          switch (bo->op()) {
          case BOT_LE: return Ranges::less(ir0,ir1);
          case BOT_LQ: return Ranges::lessEq(ir0,ir1);
          case BOT_GR: return Ranges::less(ir1,ir0);
          case BOT_GQ: return Ranges::lessEq(ir1,ir0);
          case BOT_EQ: return Ranges::equal(ir0,ir1);
          case BOT_NQ: return !Ranges::equal(ir0,ir1);
          case BOT_SUBSET: return Ranges::subset(ir0,ir1);
          case BOT_SUPERSET: return Ranges::subset(ir1,ir0);
          default:
            throw EvalError(env, e->loc(),"not a bool expression", bo->opToString());
          }
        } else if (lhs->type().isstring() && rhs->type().isstring()) {
          GCLock lock;
          std::string s0 = eval_string(env,lhs,om); // TODO
          std::string s1 = eval_string(env,rhs,om);
          switch (bo->op()) {
            case BOT_EQ: return s0==s1;
            case BOT_NQ: return s0!=s1;
            case BOT_LE: return s0<s1;
            case BOT_LQ: return s0<=s1;
            case BOT_GR: return s0>s1;
            case BOT_GQ: return s0>=s1;
            default:
              throw EvalError(env, e->loc(),"not a bool expression", bo->opToString());
          }
        } else if (bo->op()==BOT_EQ && lhs->type().isann()) {
          return Expression::equal(lhs, rhs);
        } else if (bo->op()==BOT_EQ && lhs->type().dim() > 0 &&
                   rhs->type().dim() > 0) {
          ArrayLit* al0 = eval_array_lit(env,lhs,om);
          ArrayLit* al1 = eval_array_lit(env,rhs,om);
          if (al0->v().size() != al1->v().size())
            return false;
          for (unsigned int i=0; i<al0->v().size(); i++) {
            if (!Expression::equal(eval_par(env,al0->v()[i],om), eval_par(env,al1->v()[i])),om) {
              return false;
            }
          }
          return true;
        } else {
          throw EvalError(env, e->loc(), "not a bool expression", bo->opToString());
        }
      }
      break;
    case Expression::E_UNOP:
      {
        UnOp* uo = e->cast<UnOp>();
        bool v0 = eval_bool(env,uo->e(),om);
        switch (uo->op()) {
        case UOT_NOT: return !v0;
        default:
          assert(false);
          throw EvalError(env, e->loc(),"not a bool expression", uo->opToString());
        }
      }
      break;
    case Expression::E_CALL:
      {
        Call* ce = e->cast<Call>();
        if (ce->decl()==NULL)
          throw EvalError(env, e->loc(), "undeclared function", ce->id());
        
        if (ce->decl()->_builtins.b)
          return ce->decl()->_builtins.b(env,ce);

        if (ce->decl()->_builtins.e)
          return eval_bool(env,ce->decl()->_builtins.e(env,ce),om);

        if (ce->decl()->e()==NULL)
          throw EvalError(env, ce->loc(), "internal error: missing builtin '"+ce->id().str()+"'");
        
        return eval_call<EvalBoolVal>(env,ce,om); //TODO
      }
      break;
    case Expression::E_LET:
      {
        Let* l = e->cast<Let>();
        l->pushbindings();
        bool ret = eval_bool(env,l->in(),om);
        l->popbindings();
        return ret;
      }
      break;
    default: assert(false); return false;
    }
  }

  IntSetVal* eval_boolset(EnvI& env, Expression* e, bool om) {
    switch (e->eid()) {
      case Expression::E_SETLIT:
      {
        SetLit* sl = e->cast<SetLit>();
        if (sl->isv())
          return sl->isv();
        std::vector<IntVal> vals(sl->v().size());
        for (unsigned int i=0; i<sl->v().size(); i++)
          vals[i] = eval_bool(env,sl->v()[i],om);
        return IntSetVal::a(vals);
      }
      case Expression::E_BOOLLIT:
      case Expression::E_INTLIT:
      case Expression::E_FLOATLIT:
      case Expression::E_STRINGLIT:
      case Expression::E_ANON:
      case Expression::E_TIID:
      case Expression::E_VARDECL:
      case Expression::E_TI:
      case Expression::E_UNOP:
        throw EvalError(env, e->loc(),"not a set of bool expression");
        break;
      case Expression::E_ARRAYLIT:
      {
        ArrayLit* al = e->cast<ArrayLit>();
        std::vector<IntVal> vals(al->v().size());
        for (unsigned int i=0; i<al->v().size(); i++)
          vals[i] = eval_bool(env,al->v()[i]);
        return IntSetVal::a(vals);
      }
        break;
      case Expression::E_COMP:
      {
        Comprehension* c = e->cast<Comprehension>();
        std::vector<IntVal> a = eval_comp<EvalIntVal>(env,c,om);
        return IntSetVal::a(a);
      }
      case Expression::E_ID:
      {
        GCLock lock;
        return eval_id<EvalBoolSetLit>(env,e,om)->isv();
      }
        break;
      case Expression::E_ARRAYACCESS:
      {
        GCLock lock;
        return eval_boolset(env,eval_arrayaccess(env,e->cast<ArrayAccess>(),om),om);
      }
        break;
      case Expression::E_ITE:
      {
        ITE* ite = e->cast<ITE>();
        for (int i=0; i<ite->size(); i++) {
          if (eval_bool(env,ite->e_if(i),om))
            return eval_boolset(env,ite->e_then(i),om);
        }
        return eval_boolset(env,ite->e_else(),om);
      }
        break;
      case Expression::E_BINOP:
      {
        BinOp* bo = e->cast<BinOp>();
        Expression* lhs = eval_par(env, bo->lhs(),om);
        Expression* rhs = eval_par(env, bo->rhs(),om);
        if (lhs->type().isintset() && rhs->type().isintset()) {
          IntSetVal* v0 = eval_boolset(env,lhs,om);
          IntSetVal* v1 = eval_boolset(env,rhs,om);
          IntSetRanges ir0(v0);
          IntSetRanges ir1(v1);
          switch (bo->op()) {
            case BOT_UNION:
            {
              Ranges::Union<IntSetRanges,IntSetRanges> u(ir0,ir1);
              return IntSetVal::ai(u);
            }
            case BOT_DIFF:
            {
              Ranges::Diff<IntSetRanges,IntSetRanges> u(ir0,ir1);
              return IntSetVal::ai(u);
            }
            case BOT_SYMDIFF:
            {
              Ranges::Union<IntSetRanges,IntSetRanges> u(ir0,ir1);
              Ranges::Inter<IntSetRanges,IntSetRanges> i(ir0,ir1);
              Ranges::Diff<Ranges::Union<IntSetRanges,IntSetRanges>,
              Ranges::Inter<IntSetRanges,IntSetRanges>> sd(u,i);
              return IntSetVal::ai(sd);
            }
            case BOT_INTERSECT:
            {
              Ranges::Inter<IntSetRanges,IntSetRanges> u(ir0,ir1);
              return IntSetVal::ai(u);
            }
            default: throw EvalError(env, e->loc(),"not a set of bool expression", bo->opToString());
          }
        } else if (lhs->type().isbool() && rhs->type().isbool()) {
          if (bo->op() != BOT_DOTDOT)
            throw EvalError(env, e->loc(), "not a set of bool expression", bo->opToString());
          return IntSetVal::a(eval_bool(env,lhs,om),
                              eval_bool(env,rhs,om));
        } else {
          throw EvalError(env, e->loc(), "not a set of bool expression", bo->opToString());
        }
      }
        break;
      case Expression::E_CALL:
      {
        Call* ce = e->cast<Call>();
        if (ce->decl()==NULL)
          throw EvalError(env, e->loc(), "undeclared function", ce->id());
        
        if (ce->decl()->_builtins.s)
          return ce->decl()->_builtins.s(env,ce);
        
        if (ce->decl()->_builtins.e)
          return eval_boolset(env,ce->decl()->_builtins.e(env,ce),om);
        
        if (ce->decl()->e()==NULL)
          throw EvalError(env, ce->loc(), "internal error: missing builtin '"+ce->id().str()+"'");
        
        return eval_call<EvalBoolSet>(env,ce,om);
      }
        break;
      case Expression::E_LET:
      {
        Let* l = e->cast<Let>();
        l->pushbindings();
        IntSetVal* ret = eval_boolset(env,l->in(),om);
        l->popbindings();
        return ret;
      }
        break;
      default: assert(false); return NULL;
    }
  }
  
  IntVal eval_int(EnvI& env,Expression* e, bool om) {    
    if (e->type().isbool()) {
      return eval_bool(env,e,om);
    }
    if (IntLit* il = e->dyn_cast<IntLit>()) {
      return il->v();
    }
    CallStackItem csi(env,e);
    try {
      switch (e->eid()) {
        case Expression::E_FLOATLIT:
        case Expression::E_BOOLLIT:
        case Expression::E_STRINGLIT:
        case Expression::E_ANON:
        case Expression::E_TIID:
        case Expression::E_SETLIT:
        case Expression::E_ARRAYLIT:
        case Expression::E_COMP:
        case Expression::E_VARDECL:
        case Expression::E_TI:
          throw EvalError(env, e->loc(),"not an integer expression");
          break;
        case Expression::E_ID:
        {
          GCLock lock;                    
          return eval_id<EvalIntLit>(env,e,om)->v();
        }
          break;
        case Expression::E_ARRAYACCESS:
        {
          GCLock lock;
          return eval_int(env,eval_arrayaccess(env,e->cast<ArrayAccess>(),om),om);
        }
          break;
        case Expression::E_ITE:
        {
          ITE* ite = e->cast<ITE>();
          for (int i=0; i<ite->size(); i++) {
            if (eval_bool(env,ite->e_if(i),om))
              return eval_int(env,ite->e_then(i),om);
          }
          return eval_int(env,ite->e_else(),om);
        }
          break;
        case Expression::E_BINOP:
        {
          BinOp* bo = e->cast<BinOp>();
          IntVal v0 = eval_int(env,bo->lhs(),om);
          IntVal v1 = eval_int(env,bo->rhs(),om);
          switch (bo->op()) {
            case BOT_PLUS: return v0+v1;
            case BOT_MINUS: return v0-v1;
            case BOT_MULT: return v0*v1;
            case BOT_IDIV:
              if (v1==0)
                throw EvalError(env, e->loc(),"division by zero");
              return v0 / v1;
            case BOT_MOD:
              if (v1==0)
                throw EvalError(env, e->loc(),"division by zero");
              return v0 % v1;
            default: throw EvalError(env, e->loc(),"not an integer expression", bo->opToString());
          }
        }
          break;
        case Expression::E_UNOP:
        {
          UnOp* uo = e->cast<UnOp>();
          IntVal v0 = eval_int(env,uo->e(),om);
          switch (uo->op()) {
            case UOT_PLUS: return v0;
            case UOT_MINUS: return -v0;
            default: throw EvalError(env, e->loc(),"not an integer expression", uo->opToString());
          }
        }
          break;
        case Expression::E_CALL:
        {
          Call* ce = e->cast<Call>();
          if (ce->decl()==NULL)
            throw EvalError(env, e->loc(), "undeclared function", ce->id());
          if (ce->decl()->_builtins.i)
            return ce->decl()->_builtins.i(env,ce);
          
          if (ce->decl()->_builtins.e)
            return eval_int(env,ce->decl()->_builtins.e(env,ce),om);
          
          if (ce->decl()->e()==NULL)
            throw EvalError(env, ce->loc(), "internal error: missing builtin '"+ce->id().str()+"'");
          
          return eval_call<EvalIntVal>(env,ce,om);
        }
          break;
        case Expression::E_LET:
        {
          Let* l = e->cast<Let>();
          l->pushbindings();
          IntVal ret = eval_int(env,l->in(),om);
          l->popbindings();
          return ret;
        }
          break;
        default: assert(false); return 0;
      }
    } catch (ArithmeticError& err) {
      throw EvalError(env, e->loc(), err.msg());
    }
  }

  FloatVal eval_float(EnvI& env, Expression* e, bool om) {
    if (e->type().isint()) {
      return eval_int(env,e,om).toInt();
    } else if (e->type().isbool()) {
      return eval_bool(env,e,om);
    }
    if (FloatLit* fl = e->dyn_cast<FloatLit>()) {
      return fl->v();
    }
    CallStackItem csi(env,e);
    switch (e->eid()) {
      case Expression::E_INTLIT:
      case Expression::E_BOOLLIT:
      case Expression::E_STRINGLIT:
      case Expression::E_ANON:
      case Expression::E_TIID:
      case Expression::E_SETLIT:
      case Expression::E_ARRAYLIT:
      case Expression::E_COMP:
      case Expression::E_VARDECL:
      case Expression::E_TI:
        throw EvalError(env, e->loc(),"not a float expression");
        break;
      case Expression::E_ID:
      {
        GCLock lock;
        return eval_id<EvalFloatLit>(env,e,om)->v();
      }
        break;
      case Expression::E_ARRAYACCESS:
      {
        GCLock lock;
        return eval_float(env,eval_arrayaccess(env,e->cast<ArrayAccess>(),om),om);
      }
        break;
      case Expression::E_ITE:
      {
        ITE* ite = e->cast<ITE>();
        for (int i=0; i<ite->size(); i++) {
          if (eval_bool(env,ite->e_if(i),om))
            return eval_float(env,ite->e_then(i),om);
        }
        return eval_float(env,ite->e_else(),om);
      }
        break;
      case Expression::E_BINOP:
      {
        BinOp* bo = e->cast<BinOp>();
        FloatVal v0 = eval_float(env,bo->lhs(),om);
        FloatVal v1 = eval_float(env,bo->rhs(),om);
        switch (bo->op()) {
          case BOT_PLUS: return v0+v1;
          case BOT_MINUS: return v0-v1;
          case BOT_MULT: return v0*v1;
          case BOT_DIV:
            if (v1==0.0)
              throw EvalError(env, e->loc(),"division by zero");
            return v0 / v1;
          default: throw EvalError(env, e->loc(),"not a float expression", bo->opToString());
        }
      }
        break;
      case Expression::E_UNOP:
      {
        UnOp* uo = e->cast<UnOp>();
        FloatVal v0 = eval_float(env,uo->e(),om);
        switch (uo->op()) {
          case UOT_PLUS: return v0;
          case UOT_MINUS: return -v0;
          default: throw EvalError(env, e->loc(),"not a float expression", uo->opToString());
        }
      }
        break;
      case Expression::E_CALL:
      {
        Call* ce = e->cast<Call>();
        if (ce->decl()==NULL)
          throw EvalError(env, e->loc(), "undeclared function", ce->id());
        if (ce->decl()->_builtins.f)
          return ce->decl()->_builtins.f(env,ce);
        
        if (ce->decl()->_builtins.e)
          return eval_float(env,ce->decl()->_builtins.e(env,ce),om);

        if (ce->decl()->e()==NULL)
          throw EvalError(env, ce->loc(), "internal error: missing builtin '"+ce->id().str()+"'");

        return eval_call<EvalFloatVal>(env,ce,om);
      }
        break;
      case Expression::E_LET:
      {
        Let* l = e->cast<Let>();
        l->pushbindings();
        FloatVal ret = eval_float(env,l->in(),om);
        l->popbindings();
        return ret;
      }
        break;
      default: assert(false); return 0.0;
    }
  }

  std::string eval_string(EnvI& env, Expression* e, bool om) {
    switch (e->eid()) {
      case Expression::E_STRINGLIT:
        return e->cast<StringLit>()->v().str();
      case Expression::E_FLOATLIT:
      case Expression::E_INTLIT:
      case Expression::E_BOOLLIT:
      case Expression::E_ANON:
      case Expression::E_TIID:
      case Expression::E_SETLIT:
      case Expression::E_ARRAYLIT:
      case Expression::E_COMP:
      case Expression::E_VARDECL:
      case Expression::E_TI:
        throw EvalError(env, e->loc(),"not a string expression");
        break;
      case Expression::E_ID:
      {
        GCLock lock;
        return eval_id<EvalStringLit>(env,e,om)->v().str();
      }
        break;
      case Expression::E_ARRAYACCESS:
      {
        GCLock lock;
        return eval_string(env,eval_arrayaccess(env,e->cast<ArrayAccess>(),om),om);
      }
        break;
      case Expression::E_ITE:
      {
        ITE* ite = e->cast<ITE>();
        for (int i=0; i<ite->size(); i++) {
          if (eval_bool(env,ite->e_if(i),om))
            return eval_string(env,ite->e_then(i),om);
        }
        return eval_string(env,ite->e_else(),om);
      }
        break;
      case Expression::E_BINOP:
      {
        BinOp* bo = e->cast<BinOp>();
        std::string v0 = eval_string(env,bo->lhs(),om);
        std::string v1 = eval_string(env,bo->rhs(),om);
        switch (bo->op()) {
          case BOT_PLUSPLUS: return v0+v1;
          default: throw EvalError(env, e->loc(),"not a string expression", bo->opToString());
        }
      }
        break;
      case Expression::E_UNOP:
        throw EvalError(env, e->loc(),"not a string expression");
        break;
      case Expression::E_CALL:
      {
        Call* ce = e->cast<Call>();
        if (ce->decl()==NULL)
          throw EvalError(env, e->loc(), "undeclared function", ce->id());

        if (ce->decl()->_builtins.str)
          return ce->decl()->_builtins.str(env,ce);
        if (ce->decl()->_builtins.e)
          return eval_string(env,ce->decl()->_builtins.e(env,ce),om);
        
        if (ce->decl()->e()==NULL)
          throw EvalError(env, ce->loc(), "internal error: missing builtin '"+ce->id().str()+"'");
        
        return eval_call<EvalString>(env,ce,om);
      }
        break;
      case Expression::E_LET:
      {
        Let* l = e->cast<Let>();
        l->pushbindings();
        std::string ret = eval_string(env,l->in(),om);
        l->popbindings();
        return ret;
      }
        break;
      default: assert(false); return NULL;
    }
  }

  Expression* eval_par(EnvI& env, Expression* e, bool om) {
    if (e==NULL) return NULL;   
    switch (e->eid()) {
    case Expression::E_ANON:
    case Expression::E_TIID:
    case Expression::E_MODEL:
      {
        return e;
      }
    case Expression::E_COMP:
      if (e->cast<Comprehension>()->set())
        return EvalSetLit::e(env,e);
      // fall through
    case Expression::E_ARRAYLIT:
      {
        ArrayLit* al = eval_array_lit(env,e,om);
        std::vector<Expression*> args(al->v().size());
        for (unsigned int i=al->v().size(); i--;)
          args[i] = eval_par(env,al->v()[i],om);
        std::vector<std::pair<int,int> > dims(al->dims());
        for (unsigned int i=al->dims(); i--;) {
          dims[i].first = al->min(i);
          dims[i].second = al->max(i);
        }
        ArrayLit* ret = new ArrayLit(al->loc(),args,dims);
        Type t = al->type();
        if (t.isbot() && ret->v().size() > 0) {
          t.bt(ret->v()[0]->type().bt());
        }
        ret->type(t);
        return ret;
      }
    case Expression::E_VARDECL:
      {
        VarDecl* vd = e->cast<VarDecl>();
        throw EvalError(env, vd->loc(),"cannot evaluate variable declaration", vd->id()->v());
      }
    case Expression::E_TI:
      {
        TypeInst* t = e->cast<TypeInst>();
        ASTExprVec<TypeInst> r;
        if (t->ranges().size() > 0) {
          std::vector<TypeInst*> rv(t->ranges().size());
          for (unsigned int i=t->ranges().size(); i--;)
            rv[i] = static_cast<TypeInst*>(eval_par(env,t->ranges()[i],om));
          r = ASTExprVec<TypeInst>(rv);
        }
        return 
          new TypeInst(Location(),t->type(),r,eval_par(env,t->domain(),om));
      }
    case Expression::E_ID:
      {
        if (e == constants().absent)
          return e;
        Id* id = e->cast<Id>();
        if (id->decl()==NULL)
          throw EvalError(env, e->loc(),"undefined identifier", id->v());
        if (id->decl()->ti()->domain()) {
          if (BoolLit* bl = id->decl()->ti()->domain()->dyn_cast<BoolLit>())
            return bl;
          if (id->decl()->ti()->type().isint()) {
            if (SetLit* sl = id->decl()->ti()->domain()->dyn_cast<SetLit>()) {
              if (sl->isv() && sl->isv()->min()==sl->isv()->max()) {
                return IntLit::a(sl->isv()->min());
              }
            }
          } else if (id->decl()->ti()->type().isfloat()) {
            if (BinOp* bo = id->decl()->ti()->domain()->dyn_cast<BinOp>()) {
              if (bo->op()==BOT_DOTDOT && bo->lhs()->isa<FloatLit>() && bo->rhs()->isa<FloatLit>()) {
                if (bo->lhs()->cast<FloatLit>()->v() == bo->rhs()->cast<FloatLit>()->v()) {
                  return bo->lhs();
                }
              }
            }
          }
        }
        if (id->decl()->e()==NULL) {
          return id;
        } else {
          return eval_par(env,id->decl()->e(),om);
        }
      }
    case Expression::E_STRINGLIT:
      return e;
    default:
      {        
        if (e->type().dim() != 0) {                   
          ArrayLit* al = eval_array_lit(env,e,om);         
          std::vector<Expression*> args(al->v().size());
          for (unsigned int i=al->v().size(); i--;)
            args[i] = eval_par(env,al->v()[i],om);
          std::vector<std::pair<int,int> > dims(al->dims());
          for (unsigned int i=al->dims(); i--;) {
            dims[i].first = al->min(i);
            dims[i].second = al->max(i);
          }
          ArrayLit* ret = new ArrayLit(al->loc(),args,dims);
          Type t = al->type();
          if ( (t.bt()==Type::BT_BOT || t.bt()==Type::BT_TOP) && ret->v().size() > 0) {
            t.bt(ret->v()[0]->type().bt());
          }
          ret->type(t);
          return ret;
        }
        if (e->type().isintset()) {
          return EvalSetLit::e(env,e,om);
        }
        if (e->type().isboolset()) {
          return EvalBoolSetLit::e(env,e,om);
        }
        if (e->type()==Type::parint()) {
          return EvalIntLit::e(env,e,om);
        }
        if (e->type()==Type::parbool()) {
          return EvalBoolLit::e(env,e,om);
        }
        if (e->type()==Type::parfloat()) {
          return EvalFloatLit::e(env,e,om);
        }
        if (e->type()==Type::parstring()) {
          return EvalStringLit::e(env,e,om);
        }
        switch (e->eid()) {
          case Expression::E_ITE:
          {
            ITE* ite = e->cast<ITE>();
            for (int i=0; i<ite->size(); i++) {
              if (ite->e_if(i)->type()==Type::parbool()) {
                if (eval_bool(env,ite->e_if(i),om))
                  return eval_par(env,ite->e_then(i),om);
              } else {
                std::vector<Expression*> e_ifthen(ite->size()*2);
                for (int i=0; i<ite->size(); i++) {
                  e_ifthen[2*i] = eval_par(env,ite->e_if(i),om);
                  e_ifthen[2*i+1] = eval_par(env,ite->e_then(i),om);
                }
                ITE* n_ite = new ITE(ite->loc(),e_ifthen,eval_par(env,ite->e_else(),om));
                n_ite->type(ite->type());
                return n_ite;
              }
            }
            return eval_par(env,ite->e_else(),om);
          }
          case Expression::E_CALL:
          {
            Call* c = e->cast<Call>();
            if (c->decl()) {
              if (c->decl()->_builtins.e) {
                return eval_par(env,c->decl()->_builtins.e(env,c),om);
              } else {
                if (c->decl()->e()==NULL)
                  return c;
                return eval_call<EvalPar>(env,c,om);
              }
            } else {
              std::vector<Expression*> args(c->args().size());
              for (unsigned int i=0; i<args.size(); i++) {
                args[i] = eval_par(env,c->args()[i],om);
              }
              Call* nc = new Call(c->loc(),c->id(),args,c->decl());
              nc->type(c->type());
              return nc;
            }
          }
          case Expression::E_BINOP:
          {
            BinOp* bo = e->cast<BinOp>();
            BinOp* nbo = new BinOp(e->loc(),eval_par(env,bo->lhs(),om),bo->op(),eval_par(env,bo->rhs(),om));
            nbo->type(bo->type());
            return nbo;
          }
          case Expression::E_UNOP:
          {
            UnOp* uo = e->cast<UnOp>();
            UnOp* nuo = new UnOp(e->loc(),uo->op(),eval_par(env,uo->e(),om));
            nuo->type(uo->type());
            return nuo;
          }
          case Expression::E_ARRAYACCESS:
          {
            ArrayAccess* aa = e->cast<ArrayAccess>();
            for (unsigned int i=0; i<aa->idx().size(); i++) {
              if (!aa->idx()[i]->type().ispar()) {
                std::vector<Expression*> idx(aa->idx().size());
                for (unsigned int j=0; j<aa->idx().size(); j++) {
                  idx[j] = eval_par(env, aa->idx()[j],om);
                }
                ArrayAccess* aa_new = new ArrayAccess(e->loc(), eval_par(env, aa->v(),om),idx);
                aa_new->type(aa->type());
                return aa_new;
              }
            }
            return eval_par(env,eval_arrayaccess(env,aa,om),om); 
          }
          default:
            return e;
        }
      }
    }
  }
  
  class ComputeIntBounds : public EVisitor {
  public:
    typedef std::pair<IntVal,IntVal> Bounds;
    std::vector<Bounds> _bounds;
    bool valid;
    EnvI& env;
    ComputeIntBounds(EnvI& env0) : valid(true), env(env0) {}
    bool enter(Expression* e) {
      if (e->type().isann())
        return false;
      if (e->isa<VarDecl>())
        return false;
      if (e->type().dim() > 0)
        return false;
      if (e->type().ispar()) {
        if (e->type().isint()) {
          IntVal v = eval_int(env,e);
          _bounds.push_back(Bounds(v,v));
        } else {
          valid = false;
        }
        return false;
      }
      if (ITE* ite = e->dyn_cast<ITE>()) {
        Bounds itebounds(IntVal::infinity(), -IntVal::infinity());
        for (unsigned int i=0; i<ite->size(); i++) {
          if (ite->e_if(i)->type().ispar() && ite->e_if(i)->type().cv()==Type::CV_NO) {
            if (eval_bool(env, ite->e_if(i))) {
              BottomUpIterator<ComputeIntBounds> cbi(*this);
              cbi.run(ite->e_then(i));
              Bounds& back = _bounds.back();
              back.first = std::min(itebounds.first, back.first);
              back.second = std::max(itebounds.second, back.second);
              return false;
            }
          } else {
            BottomUpIterator<ComputeIntBounds> cbi(*this);
            cbi.run(ite->e_then(i));
            Bounds back = _bounds.back();
            _bounds.pop_back();
            itebounds.first = std::min(itebounds.first, back.first);
            itebounds.second = std::max(itebounds.second, back.second);
          }
        }
        BottomUpIterator<ComputeIntBounds> cbi(*this);
        cbi.run(ite->e_else());
        Bounds& back = _bounds.back();
        back.first = std::min(itebounds.first, back.first);
        back.second = std::max(itebounds.second, back.second);
        return false;
      }
      return e->type().isint();
    }
    /// Visit integer literal
    void vIntLit(const IntLit& i) {
      _bounds.push_back(Bounds(i.v(),i.v()));
    }
    /// Visit floating point literal
    void vFloatLit(const FloatLit&) {
      valid = false;
      _bounds.push_back(Bounds(0,0));
    }
    /// Visit Boolean literal
    void vBoolLit(const BoolLit&) {
      valid = false;
      _bounds.push_back(Bounds(0,0));
    }
    /// Visit set literal
    void vSetLit(const SetLit&) {
      valid = false;
      _bounds.push_back(Bounds(0,0));
    }
    /// Visit string literal
    void vStringLit(const StringLit&) {
      valid = false;
      _bounds.push_back(Bounds(0,0));
    }
    /// Visit identifier
    void vId(const Id& id) {
      VarDecl* vd = id.decl();
      while (vd->flat() && vd->flat() != vd)
        vd = vd->flat();
      if (vd->ti()->domain()) {
        GCLock lock;
        IntSetVal* isv = eval_intset(env,vd->ti()->domain());
        if (isv->size()==0) {
          valid = false;
          _bounds.push_back(Bounds(0,0));
        } else {
          _bounds.push_back(Bounds(isv->min(0),isv->max(isv->size()-1)));
        }
      } else {
        if (vd->e()) {
          BottomUpIterator<ComputeIntBounds> cbi(*this);
          cbi.run(vd->e());
        } else {
          _bounds.push_back(Bounds(-IntVal::infinity(),IntVal::infinity()));
        }
      }
    }
    /// Visit anonymous variable
    void vAnonVar(const AnonVar& v) {
      valid = false;
      _bounds.push_back(Bounds(0,0));
    }
    /// Visit array literal
    void vArrayLit(const ArrayLit& al) {
    }
    /// Visit array access
    void vArrayAccess(ArrayAccess& aa) {
      bool parAccess = true;
      for (unsigned int i=aa.idx().size(); i--;) {
        _bounds.pop_back();
        if (!aa.idx()[i]->type().ispar()) {
          parAccess = false;
        }
      }
      if (Id* id = aa.v()->dyn_cast<Id>()) {
        while (id->decl()->e() && id->decl()->e()->isa<Id>()) {
          id = id->decl()->e()->cast<Id>();
        }
        if (parAccess && id->decl()->e() && id->decl()->e()->isa<ArrayLit>()) {
          bool success;
          Expression* e = eval_arrayaccess(env,&aa, success);
          if (success) {
            BottomUpIterator<ComputeIntBounds> cbi(*this);
            cbi.run(e);
            return;
          }
        }
        if (id->decl()->ti()->domain()) {
          GCLock lock;
          IntSetVal* isv = eval_intset(env,id->decl()->ti()->domain());
          if (isv->size()>0) {
            _bounds.push_back(Bounds(isv->min(0),isv->max(isv->size()-1)));
            return;
          }
        }
      }
      valid = false;
      _bounds.push_back(Bounds(0,0));
    }
    /// Visit array comprehension
    void vComprehension(const Comprehension& c) {
      valid = false;
      _bounds.push_back(Bounds(0,0));
    }
    /// Visit if-then-else
    void vITE(const ITE& ite) {
      valid = false;
      _bounds.push_back(Bounds(0,0));
    }
    /// Visit binary operator
    void vBinOp(const BinOp& bo) {
      Bounds b1 = _bounds.back(); _bounds.pop_back();
      Bounds b0 = _bounds.back(); _bounds.pop_back();
      if (!b1.first.isFinite() || !b1.second.isFinite() || !b0.first.isFinite() || !b0.second.isFinite()) {
        valid = false;
        _bounds.push_back(Bounds(0,0));
      } else {
        switch (bo.op()) {
          case BOT_PLUS:
            _bounds.push_back(Bounds(b0.first+b1.first,b0.second+b1.second));
            break;
          case BOT_MINUS:
            _bounds.push_back(Bounds(b0.first-b1.second,b0.second-b1.first));
            break;
          case BOT_MULT:
          {
            IntVal x0 = b0.first*b1.first;
            IntVal x1 = b0.first*b1.second;
            IntVal x2 = b0.second*b1.first;
            IntVal x3 = b0.second*b1.second;
            IntVal m = std::min(x0,std::min(x1,std::min(x2,x3)));
            IntVal n = std::max(x0,std::max(x1,std::max(x2,x3)));
            _bounds.push_back(Bounds(m,n));
          }
            break;
          case BOT_IDIV:
          {
            IntVal b0f = b0.first==0 ? 1 : b0.first;
            IntVal b0s = b0.second==0 ? -1 : b0.second;
            IntVal b1f = b1.first==0 ? 1 : b1.first;
            IntVal b1s = b1.second==0 ? -1 : b1.second;
            IntVal x0 = b0f / b1f;
            IntVal x1 = b0f / b1s;
            IntVal x2 = b0s / b1f;
            IntVal x3 = b0s / b1s;
            IntVal m = std::min(x0,std::min(x1,std::min(x2,x3)));
            IntVal n = std::max(x0,std::max(x1,std::max(x2,x3)));
            _bounds.push_back(Bounds(m,n));
          }
            break;
          case BOT_MOD:
          {
            IntVal b0f = b0.first==0 ? 1 : b0.first;
            IntVal b0s = b0.second==0 ? -1 : b0.second;
            IntVal b1f = b1.first==0 ? 1 : b1.first;
            IntVal b1s = b1.second==0 ? -1 : b1.second;
            IntVal x0 = b0f % b1f;
            IntVal x1 = b0f % b1s;
            IntVal x2 = b0s % b1f;
            IntVal x3 = b0s % b1s;
            IntVal m = std::min(x0,std::min(x1,std::min(x2,x3)));
            IntVal n = std::max(x0,std::max(x1,std::max(x2,x3)));
            _bounds.push_back(Bounds(m,n));
          }
            break;
          case BOT_DIV:
          case BOT_LE:
          case BOT_LQ:
          case BOT_GR:
          case BOT_GQ:
          case BOT_EQ:
          case BOT_NQ:
          case BOT_IN:
          case BOT_SUBSET:
          case BOT_SUPERSET:
          case BOT_UNION:
          case BOT_DIFF:
          case BOT_SYMDIFF:
          case BOT_INTERSECT:
          case BOT_PLUSPLUS:
          case BOT_EQUIV:
          case BOT_IMPL:
          case BOT_RIMPL:
          case BOT_OR:
          case BOT_AND:
          case BOT_XOR:
          case BOT_DOTDOT:
            valid = false;
            _bounds.push_back(Bounds(0,0));
        }
      }
    }
    /// Visit unary operator
    void vUnOp(const UnOp& uo) {
      switch (uo.op()) {
      case UOT_PLUS:
        break;
      case UOT_MINUS:
          _bounds.back().first = -_bounds.back().first;
          _bounds.back().second = -_bounds.back().second;
          std::swap(_bounds.back().first, _bounds.back().second);
        break;
      case UOT_NOT:
        valid = false;
      }
    }
    /// Visit call
    void vCall(Call& c) {
      if (c.id() == constants().ids.lin_exp || c.id() == constants().ids.sum) {
        bool le = c.id() == constants().ids.lin_exp;
        ArrayLit* coeff = le ? eval_array_lit(env,c.args()[0]): NULL;
        if (c.args()[le ? 1 : 0]->type().isopt()) {
          valid = false;
          _bounds.push_back(Bounds(0,0));
          return;
        }
        ArrayLit* al = eval_array_lit(env,c.args()[le ? 1 : 0]);
        IntVal d = le ? c.args()[2]->cast<IntLit>()->v() : 0;
        int stacktop = _bounds.size();
        for (unsigned int i=al->v().size(); i--;) {
          BottomUpIterator<ComputeIntBounds> cbi(*this);
          cbi.run(al->v()[i]);
          if (!valid) {
            for (unsigned int j=al->v().size()-1; j>i; j--)
              _bounds.pop_back();
            return;
          }
        }
        assert(stacktop+al->v().size()==_bounds.size());
        IntVal lb = d;
        IntVal ub = d;
        for (unsigned int i=0; i<al->v().size(); i++) {
          Bounds b = _bounds.back(); _bounds.pop_back();
          IntVal cv = le ? eval_int(env,coeff->v()[i]) : 1;
          if (cv > 0) {
            if (b.first.isFinite()) {
              if (lb.isFinite()) {
                lb += cv*b.first;
              }
            } else {
              lb = b.first;
            }
            if (b.second.isFinite()) {
              if (ub.isFinite()) {
                ub += cv*b.second;
              }
            } else {
              ub = b.second;
            }
          } else {
            if (b.second.isFinite()) {
              if (lb.isFinite()) {
                lb += cv*b.second;
              }
            } else {
              lb = -b.second;
            }
            if (b.first.isFinite()) {
              if (ub.isFinite()) {
                ub += cv*b.first;
              }
            } else {
              ub = -b.first;
            }
          }
        }
        _bounds.push_back(Bounds(lb,ub));
      } else if (c.id() == "card") {
        if (IntSetVal* isv = compute_intset_bounds(env,c.args()[0])) {
          IntSetRanges isr(isv);
          _bounds.push_back(Bounds(0,Ranges::size(isr)));
        } else {
          valid = false;
          _bounds.push_back(Bounds(0,0));
        }
      } else if (c.id() == "int_times") {
        Bounds b1 = _bounds.back(); _bounds.pop_back();
        Bounds b0 = _bounds.back(); _bounds.pop_back();
        if (!b1.first.isFinite() || !b1.second.isFinite() || !b0.first.isFinite() || !b0.second.isFinite()) {
          valid = false;
          _bounds.push_back(Bounds(0,0));
        } else {
          IntVal x0 = b0.first*b1.first;
          IntVal x1 = b0.first*b1.second;
          IntVal x2 = b0.second*b1.first;
          IntVal x3 = b0.second*b1.second;
          IntVal m = std::min(x0,std::min(x1,std::min(x2,x3)));
          IntVal n = std::max(x0,std::max(x1,std::max(x2,x3)));
          _bounds.push_back(Bounds(m,n));
        }
      } else if (c.id() == constants().ids.bool2int) {
          _bounds.push_back(Bounds(0,1));
      } else if (c.id() == "abs") {
        Bounds b0 = _bounds.back();
        if (b0.first < 0) {
          _bounds.pop_back();
          if (b0.second < 0)
            _bounds.push_back(Bounds(-b0.second,-b0.first));
          else
            _bounds.push_back(Bounds(0,std::max(-b0.first,b0.second)));
        }
      } else {
        valid = false;
        _bounds.push_back(Bounds(0,0));
      }
    }
    /// Visit let
    void vLet(const Let& l) {
      valid = false;
      _bounds.push_back(Bounds(0,0));
    }
    /// Visit variable declaration
    void vVarDecl(const VarDecl& vd) {
      valid = false;
      _bounds.push_back(Bounds(0,0));
    }
    /// Visit annotation
    void vAnnotation(const Annotation& e) {
      valid = false;
      _bounds.push_back(Bounds(0,0));
    }
    /// Visit type inst
    void vTypeInst(const TypeInst& e) {
      valid = false;
      _bounds.push_back(Bounds(0,0));
    }
    /// Visit TIId
    void vTIId(const TIId& e) {
      valid = false;
      _bounds.push_back(Bounds(0,0));
    }
  };

  IntBounds compute_int_bounds(EnvI& env, Expression* e) {
    ComputeIntBounds cb(env);
    BottomUpIterator<ComputeIntBounds> cbi(cb);
    cbi.run(e);
    if (cb.valid) {
      assert(cb._bounds.size() > 0);
      return IntBounds(cb._bounds.back().first,cb._bounds.back().second,true);
    } else {
      return IntBounds(0,0,false);
    }
  }

  class ComputeFloatBounds : public EVisitor {
  protected:
    typedef std::pair<FloatVal,FloatVal> FBounds;
  public:
    std::vector<FBounds> _bounds;
    bool valid;
    EnvI& env;
    ComputeFloatBounds(EnvI& env0) : valid(true), env(env0) {}
    bool enter(Expression* e) {
      if (e->type().isann())
        return false;
      if (e->isa<VarDecl>())
        return false;
      if (e->type().dim() > 0)
        return false;
      if (e->type().ispar()) {
        if (e->type().isfloat()) {
          FloatVal v = eval_float(env,e);
          _bounds.push_back(FBounds(v,v));
        } else {
          valid = false;
        }
        return false;
      } else {
        return e->type().isfloat();
      }
    }
    /// Visit integer literal
    void vIntLit(const IntLit& i) {
      valid = false;
      _bounds.push_back(FBounds(0.0,0.0));
    }
    /// Visit floating point literal
    void vFloatLit(const FloatLit& f) {
      _bounds.push_back(FBounds(f.v(),f.v()));
    }
    /// Visit Boolean literal
    void vBoolLit(const BoolLit&) {
      valid = false;
      _bounds.push_back(FBounds(0.0,0.0));
    }
    /// Visit set literal
    void vSetLit(const SetLit&) {
      valid = false;
      _bounds.push_back(FBounds(0.0,0.0));
    }
    /// Visit string literal
    void vStringLit(const StringLit&) {
      valid = false;
      _bounds.push_back(FBounds(0.0,0.0));
    }
    /// Visit identifier
    void vId(const Id& id) {
      VarDecl* vd = id.decl();
      while (vd->flat() && vd->flat() != vd)
        vd = vd->flat();
      if (vd->ti()->domain()) {
        BinOp* bo = vd->ti()->domain()->cast<BinOp>();
        assert(bo->op() == BOT_DOTDOT);
        _bounds.push_back(FBounds(eval_float(env,bo->lhs()),eval_float(env,bo->rhs())));
      } else {
        if (vd->e()) {
          BottomUpIterator<ComputeFloatBounds> cbi(*this);
          cbi.run(vd->e());
        } else {
          valid = false;
          _bounds.push_back(FBounds(0,0));
        }
      }
    }
    /// Visit anonymous variable
    void vAnonVar(const AnonVar& v) {
      valid = false;
      _bounds.push_back(FBounds(0.0,0.0));
    }
    /// Visit array literal
    void vArrayLit(const ArrayLit& al) {
    }
    /// Visit array access
    void vArrayAccess(ArrayAccess& aa) {
      bool parAccess = true;
      for (unsigned int i=aa.idx().size(); i--;) {
        if (!aa.idx()[i]->type().ispar()) {
          parAccess = false;
        }
      }
      if (Id* id = aa.v()->dyn_cast<Id>()) {
        while (id->decl()->e() && id->decl()->e()->isa<Id>()) {
          id = id->decl()->e()->cast<Id>();
        }
        if (parAccess && id->decl()->e() && id->decl()->e()->isa<ArrayLit>()) {
          bool success;
          Expression* e = eval_arrayaccess(env,&aa, success);
          if (success) {
            BottomUpIterator<ComputeFloatBounds> cbi(*this);
            cbi.run(e);
            return;
          }
        }
        if (id->decl()->ti()->domain()) {
          BinOp* bo = id->decl()->ti()->domain()->cast<BinOp>();
          assert(bo->op() == BOT_DOTDOT);
          FBounds b(eval_float(env,bo->lhs()),eval_float(env,bo->rhs()));
          _bounds.push_back(b);
          return;
        }
      }
      valid = false;
      _bounds.push_back(FBounds(0.0,0.0));
    }
    /// Visit array comprehension
    void vComprehension(const Comprehension& c) {
      valid = false;
      _bounds.push_back(FBounds(0.0,0.0));
    }
    /// Visit if-then-else
    void vITE(const ITE& ite) {
      valid = false;
      _bounds.push_back(FBounds(0.0,0.0));
    }
    /// Visit binary operator
    void vBinOp(const BinOp& bo) {
      FBounds b1 = _bounds.back(); _bounds.pop_back();
      FBounds b0 = _bounds.back(); _bounds.pop_back();
      switch (bo.op()) {
        case BOT_PLUS:
          _bounds.push_back(FBounds(b0.first+b1.first,b0.second+b1.second));
          break;
        case BOT_MINUS:
          _bounds.push_back(FBounds(b0.first-b1.second,b0.second-b1.first));
          break;
        case BOT_MULT:
        {
          FloatVal x0 = b0.first*b1.first;
          FloatVal x1 = b0.first*b1.second;
          FloatVal x2 = b0.second*b1.first;
          FloatVal x3 = b0.second*b1.second;
          FloatVal m = std::min(x0,std::min(x1,std::min(x2,x3)));
          FloatVal n = std::max(x0,std::max(x1,std::max(x2,x3)));
          _bounds.push_back(FBounds(m,n));
        }
          break;
        case BOT_DIV:
        case BOT_IDIV:
        case BOT_MOD:
        case BOT_LE:
        case BOT_LQ:
        case BOT_GR:
        case BOT_GQ:
        case BOT_EQ:
        case BOT_NQ:
        case BOT_IN:
        case BOT_SUBSET:
        case BOT_SUPERSET:
        case BOT_UNION:
        case BOT_DIFF:
        case BOT_SYMDIFF:
        case BOT_INTERSECT:
        case BOT_PLUSPLUS:
        case BOT_EQUIV:
        case BOT_IMPL:
        case BOT_RIMPL:
        case BOT_OR:
        case BOT_AND:
        case BOT_XOR:
        case BOT_DOTDOT:
          valid = false;
          _bounds.push_back(FBounds(0,0));
      }
    }
    /// Visit unary operator
    void vUnOp(const UnOp& uo) {
      switch (uo.op()) {
        case UOT_PLUS:
          break;
        case UOT_MINUS:
          _bounds.back().first = -_bounds.back().first;
          _bounds.back().second = -_bounds.back().second;
          break;
        case UOT_NOT:
          valid = false;
          _bounds.push_back(FBounds(0.0,0.0));
      }
    }
    /// Visit call
    void vCall(Call& c) {
      if (c.id() == constants().ids.lin_exp || c.id() == constants().ids.sum) {
        bool le = c.id() == constants().ids.lin_exp;
        ArrayLit* coeff = le ? eval_array_lit(env,c.args()[0]): NULL;
        if (c.args()[le ? 1 : 0]->type().isopt()) {
          valid = false;
          _bounds.push_back(FBounds(0.0,0.0));
          return;
        }
        ArrayLit* al = eval_array_lit(env,c.args()[le ? 1 : 0]);
        FloatVal d = le ? c.args()[2]->cast<FloatLit>()->v() : 0.0;
        int stacktop = _bounds.size();
        for (unsigned int i=al->v().size(); i--;) {
          BottomUpIterator<ComputeFloatBounds> cbi(*this);
          cbi.run(al->v()[i]);
          if (!valid)
            return;
        }
        assert(stacktop+al->v().size()==_bounds.size());
        FloatVal lb = d;
        FloatVal ub = d;
        for (unsigned int i=0; i<al->v().size(); i++) {
          FBounds b = _bounds.back(); _bounds.pop_back();
          FloatVal cv = le ? eval_float(env,coeff->v()[i]) : 1.0;
          if (cv > 0) {
            lb += cv*b.first;
            ub += cv*b.second;
          } else {
            lb += cv*b.second;
            ub += cv*b.first;
          }
        }
        _bounds.push_back(FBounds(lb,ub));
      } else if (c.id() == "float_times") {
        BottomUpIterator<ComputeFloatBounds> cbi(*this);
        cbi.run(c.args()[0]);
        cbi.run(c.args()[1]);
        FBounds b1 = _bounds.back(); _bounds.pop_back();
        FBounds b0 = _bounds.back(); _bounds.pop_back();
        FloatVal x0 = b0.first*b1.first;
        FloatVal x1 = b0.first*b1.second;
        FloatVal x2 = b0.second*b1.first;
        FloatVal x3 = b0.second*b1.second;
        FloatVal m = std::min(x0,std::min(x1,std::min(x2,x3)));
        FloatVal n = std::max(x0,std::max(x1,std::max(x2,x3)));
        _bounds.push_back(FBounds(m,n));
      } else if (c.id() == "int2float") {
        ComputeIntBounds ib(env);
        BottomUpIterator<ComputeIntBounds> cbi(ib);
        cbi.run(c.args()[0]);
        if (!ib.valid)
          valid = false;
        ComputeIntBounds::Bounds result = ib._bounds.back();
        if (!result.first.isFinite() || !result.second.isFinite()) {
          valid = false;
          _bounds.push_back(FBounds(0.0,0.0));
        } else {
          _bounds.push_back(FBounds(result.first.toInt(),result.second.toInt()));
        }
      } else if (c.id() == "abs") {
        BottomUpIterator<ComputeFloatBounds> cbi(*this);
        cbi.run(c.args()[0]);
        FBounds b0 = _bounds.back();
        if (b0.first < 0) {
          _bounds.pop_back();
          if (b0.second < 0)
            _bounds.push_back(FBounds(-b0.second,-b0.first));
          else
            _bounds.push_back(FBounds(0.0,std::max(-b0.first,b0.second)));
        }
      } else {
        valid = false;
        _bounds.push_back(FBounds(0.0,0.0));
      }
    }
    /// Visit let
    void vLet(const Let& l) {
      valid = false;
      _bounds.push_back(FBounds(0.0,0.0));
    }
    /// Visit variable declaration
    void vVarDecl(const VarDecl& vd) {
      valid = false;
      _bounds.push_back(FBounds(0.0,0.0));
    }
    /// Visit annotation
    void vAnnotation(const Annotation& e) {
      valid = false;
      _bounds.push_back(FBounds(0.0,0.0));
    }
    /// Visit type inst
    void vTypeInst(const TypeInst& e) {
      valid = false;
      _bounds.push_back(FBounds(0.0,0.0));
    }
    /// Visit TIId
    void vTIId(const TIId& e) {
      valid = false;
      _bounds.push_back(FBounds(0.0,0.0));
    }
  };
  
  FloatBounds compute_float_bounds(EnvI& env, Expression* e) {
    ComputeFloatBounds cb(env);
    BottomUpIterator<ComputeFloatBounds> cbi(cb);
    cbi.run(e);
    if (cb.valid) {
      assert(cb._bounds.size() > 0);
      return FloatBounds(cb._bounds.back().first,cb._bounds.back().second,true);
    } else {
      return FloatBounds(0.0,0.0,false);
    }
  }
  
  class ComputeIntSetBounds : public EVisitor {
  public:
    std::vector<IntSetVal*> _bounds;
    bool valid;
    EnvI& env;
    ComputeIntSetBounds(EnvI& env0) : valid(true), env(env0) {}
    bool enter(Expression* e) {
      if (e->type().isann())
        return false;
      if (e->isa<VarDecl>())
        return false;
      if (e->type().dim() > 0)
        return false;
      if (!e->type().isintset())
        return false;
      if (e->type().ispar()) {
        _bounds.push_back(eval_intset(env,e));
        return false;
      } else {
        return true;
      }
    }
    /// Visit set literal
    void vSetLit(const SetLit& sl) {
      assert(sl.type().isvar());
      assert(sl.isv()==NULL);

      IntSetVal* isv = IntSetVal::a();
      for (unsigned int i=0; i<sl.v().size(); i++) {
        IntSetRanges i0(isv);
        IntBounds ib = compute_int_bounds(env,sl.v()[i]);
        if (!ib.valid || !ib.l.isFinite() || !ib.u.isFinite()) {
          valid = false;
          _bounds.push_back(NULL);
          return;
        }
        Ranges::Const cr(ib.l,ib.u);
        Ranges::Union<IntSetRanges, Ranges::Const> u(i0,cr);
        isv = IntSetVal::ai(u);
      }
      _bounds.push_back(isv);
    }
    /// Visit identifier
    void vId(const Id& id) {
      if (id.decl()->ti()->domain()) {
        _bounds.push_back(eval_intset(env,id.decl()->ti()->domain()));
      } else {
        if (id.decl()->e()) {
          BottomUpIterator<ComputeIntSetBounds> cbi(*this);
          cbi.run(id.decl()->e());
        } else {
          valid = false;
          _bounds.push_back(NULL);
        }
      }
    }
    /// Visit anonymous variable
    void vAnonVar(const AnonVar& v) {
      valid = false;
      _bounds.push_back(NULL);
    }
    /// Visit array access
    void vArrayAccess(ArrayAccess& aa) {
      bool parAccess = true;
      for (unsigned int i=aa.idx().size(); i--;) {
        if (!aa.idx()[i]->type().ispar()) {
          parAccess = false;
          break;
        }
      }
      if (Id* id = aa.v()->dyn_cast<Id>()) {
        while (id->decl()->e() && id->decl()->e()->isa<Id>()) {
          id = id->decl()->e()->cast<Id>();
        }
        if (parAccess && id->decl()->e() && id->decl()->e()->isa<ArrayLit>()) {
          bool success;
          Expression* e = eval_arrayaccess(env,&aa, success);
          if (success) {
            BottomUpIterator<ComputeIntSetBounds> cbi(*this);
            cbi.run(e);
            return;
          }
        }
        if (id->decl()->ti()->domain()) {
          _bounds.push_back(eval_intset(env,id->decl()->ti()->domain()));
          return;
        }
      }
      valid = false;
      _bounds.push_back(NULL);
    }
    /// Visit array comprehension
    void vComprehension(const Comprehension& c) {
      valid = false;
      _bounds.push_back(NULL);
    }
    /// Visit if-then-else
    void vITE(const ITE& ite) {
      valid = false;
      _bounds.push_back(NULL);
    }
    /// Visit binary operator
    void vBinOp(const BinOp& bo) {
      if (bo.op()==BOT_DOTDOT) {
        IntBounds lb = compute_int_bounds(env, bo.lhs());
        IntBounds ub = compute_int_bounds(env, bo.rhs());
        valid = valid && lb.valid && ub.valid;
        _bounds.push_back(IntSetVal::a(lb.l, ub.u));
      } else {
        IntSetVal* b1 = _bounds.back(); _bounds.pop_back();
        IntSetVal* b0 = _bounds.back(); _bounds.pop_back();
        switch (bo.op()) {
        case BOT_INTERSECT:
        case BOT_UNION:
          {
            IntSetRanges b0r(b0);
            IntSetRanges b1r(b1);
            Ranges::Union<IntSetRanges,IntSetRanges> u(b0r,b1r);
            _bounds.push_back(IntSetVal::ai(u));
          }
          break;
        case BOT_DIFF:
          {
            _bounds.push_back(b0);
          }
          break;
        case BOT_SYMDIFF:
          valid = false;
          _bounds.push_back(NULL);
          break;
        case BOT_PLUS:
        case BOT_MINUS:
        case BOT_MULT:
        case BOT_DIV:
        case BOT_IDIV:
        case BOT_MOD:
        case BOT_LE:
        case BOT_LQ:
        case BOT_GR:
        case BOT_GQ:
        case BOT_EQ:
        case BOT_NQ:
        case BOT_IN:
        case BOT_SUBSET:
        case BOT_SUPERSET:
        case BOT_PLUSPLUS:
        case BOT_EQUIV:
        case BOT_IMPL:
        case BOT_RIMPL:
        case BOT_OR:
        case BOT_AND:
        case BOT_XOR:
        case BOT_DOTDOT:
          valid = false;
          _bounds.push_back(NULL);
        }
      }
    }
    /// Visit unary operator
    void vUnOp(const UnOp& uo) {
      valid = false;
      _bounds.push_back(NULL);
    }
    /// Visit call
    void vCall(Call& c) {
      if (valid && (c.id() == "set_intersect" || c.id() == "set_union")) {
        IntSetVal* b0 = _bounds.back(); _bounds.pop_back();
        IntSetVal* b1 = _bounds.back(); _bounds.pop_back();
        IntSetRanges b0r(b0);
        IntSetRanges b1r(b1);
        Ranges::Union<IntSetRanges,IntSetRanges> u(b0r,b1r);
        _bounds.push_back(IntSetVal::ai(u));
      } else if (valid && c.id() == "set_diff") {
        IntSetVal* b0 = _bounds.back(); _bounds.pop_back();
        _bounds.pop_back(); // don't need bounds of right hand side
        _bounds.push_back(b0);
      } else {
        valid = false;
        _bounds.push_back(NULL);
      }
    }
    /// Visit let
    void vLet(const Let& l) {
      valid = false;
      _bounds.push_back(NULL);
    }
    /// Visit variable declaration
    void vVarDecl(const VarDecl& vd) {
      valid = false;
      _bounds.push_back(NULL);
    }
    /// Visit annotation
    void vAnnotation(const Annotation& e) {
      valid = false;
      _bounds.push_back(NULL);
    }
    /// Visit type inst
    void vTypeInst(const TypeInst& e) {
      valid = false;
      _bounds.push_back(NULL);
    }
    /// Visit TIId
    void vTIId(const TIId& e) {
      valid = false;
      _bounds.push_back(NULL);
    }
  };

  IntSetVal* compute_intset_bounds(EnvI& env, Expression* e) {
    ComputeIntSetBounds cb(env);
    BottomUpIterator<ComputeIntSetBounds> cbi(cb);
    cbi.run(e);
    if (cb.valid)
      return cb._bounds.back();
    else
      return NULL;  
  }

}
