/*
 *  Main authors:
 *     Kevin Leo <kevin.leo@monash.edu>
 *     Andrea Rendl <andrea.rendl@nicta.com.au>
 *     Guido Tack <guido.tack@monash.edu>
 */

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#ifndef __MINIZINC_GECODE_SOLVER_INSTANCE_HH__
#define __MINIZINC_GECODE_SOLVER_INSTANCE_HH__

#include <gecode/kernel.hh>
#include <gecode/int.hh>
#include <gecode/driver.hh>

#ifdef GECODE_HAS_SET_VARS
#include <gecode/set.hh>
#endif

#define GECODE_HAS_FLOAT_VARS
#ifdef GECODE_HAS_FLOAT_VARS
#include <gecode/float.hh>
#endif

#include <minizinc/solver_instance_base.hh>
#include <minizinc/solvers/gecode/fzn_space.hh>

namespace MiniZinc {
    
  class GecodeVariable {
  public:
    enum vartype {BOOL_TYPE,FLOAT_TYPE,INT_TYPE,SET_TYPE};
  protected:
    /// variable type
    vartype _t;
    /// the index in the iv/bv/fv/sv array in the space, depending the type _t
    unsigned int _index;
    /// the index in FznSpace::bv of the boolean variable that corresponds to the int var; if not exists then -1
    int _boolAliasIndex;
  public:
    GecodeVariable(vartype t, unsigned int index) : 
        _t(t), _index(index), _boolAliasIndex(-1) {}
        
    bool isint(void) const {
      return _t == INT_TYPE;
    }
    
    bool isbool(void) const {
      return _t == BOOL_TYPE;
    }
    
    bool isfloat(void) const {
      return _t == FLOAT_TYPE;
    }
    
    bool isset(void) const {
      return _t == SET_TYPE;
    }        
    
    bool hasBoolAlias(void) {
      return _boolAliasIndex >= 0;
    }
    
    /// set the index in FznSpace::bv of the Boolean variable that corresponds to the int variable
    void setBoolAliasIndex(int index) {
      assert(_t == INT_TYPE);
      assert(index >= 0);    
      _boolAliasIndex = index;
    }
    
    int boolAliasIndex(void) {
      return  _boolAliasIndex;
    }

    int index(void) {
      return _index;
    }
    
    Gecode::IntVar& intVar(MiniZinc::FznSpace* space) {
      assert(_t == INT_TYPE);
      assert(_index < space->iv.size());
      return space->iv[_index];
    }
    
    Gecode::BoolVar& boolVar(MiniZinc::FznSpace* space) {
      assert(_t == BOOL_TYPE);
      assert(_index < space->bv.size());
      return space->bv[_index];
    }

#ifdef GECODE_HAS_FLOAT_VARS
    Gecode::FloatVar& floatVar(MiniZinc::FznSpace* space) {
      assert(_t == FLOAT_TYPE);
      assert(_index < space->fv.size());
      return space->fv[_index];
    }
#endif

#ifdef GECODE_HAS_SET_VARS
    Gecode::SetVar& setVar(MiniZinc::FznSpace* space) {
      assert(_t == SET_TYPE);
      assert(_index < space->sv.size());
      return space->sv[_index];
    }
#endif


    
  };
  
  
  class GecodeSolver {
  public:
    typedef GecodeVariable Variable;
    typedef MiniZinc::Statistics Statistics;
  };
  
  class GecodeEngine;
  class CustomEngine;
  
  class GecodeSolverInstance : public SolverInstanceImpl<GecodeSolver> {   
  private:
    bool _print_stats;
    bool _only_range_domains;
    /// the objective variable for builtin-bab
    Id* _objVar; 

    bool _run_sac;
    bool _run_shave;
    unsigned int _pre_passes;
    unsigned int _n_max_solutions;
    unsigned int _n_found_solutions;
    Model* _flat; // TODO: do we need this? Can't we access _env->flat()?

  public:
    /// the Gecode space that will be/has been solved
    FznSpace* _current_space; 
    /// the solution (or NULL if does not exist or not yet computed)
    FznSpace* _solution;
    /// the variable declarations with output annotations
    std::vector<VarDecl*> _varsWithOutput;
    /// declaration map for processing and printing output
    //typedef std::pair<VarDecl*,Expression*> DE;
    //ASTStringMap<DE>::t _declmap;
    /// TODO: we can probably get rid of this
    UNORDERED_NAMESPACE::unordered_map<VarDecl*, std::vector<Expression*>* > arrayMap;
    /// The solver engine for regular search
    GecodeEngine* engine;
    /// the solver engine for combinators
    CustomEngine* customEngine;

    GecodeSolverInstance(Env& env, const Options& options);
    virtual ~GecodeSolverInstance(void);

    virtual SolverInstanceBase* copy(CopyMap& cmap);
    
    virtual Status next(void);  
    virtual Status best(VarDecl* obj, bool minimize, bool print);
    virtual void processFlatZinc(void);    
    virtual Status solve(void);
    /// this is the version that the combinator interpreter calls which will evoke the search engine
    virtual bool updateIntBounds(VarDecl* vd, int lb, int ub);
    /// this is the version that is called by the Gecode engine and applied to each space
    bool updateIntBounds(FznSpace* space, VarDecl* vd, int lb, int ub); 
    /// post constraints incrementally (after next() has been called); this function is called by the combinators interpreter
    virtual bool postConstraints(std::vector<Call*> cts);       
    /// add variables incrementally (after next() has been called); this function is called by the combinators interpreter
    virtual bool addVariables(const std::vector<VarDecl*>& vars);
    /// add variables incrementally to the given space (called by the engine)
    bool addVariables(FznSpace* space, const std::vector<VarDecl*>& vars);
    
    // Presolve the currently loaded model, updating variables with the same
    // names in the given Model* m.
    bool presolve(Model* m = NULL);
    bool sac(bool toFixedPoint, bool shaving);
    void print_stats();

    virtual Expression* getSolutionValue(Id* id);

    Gecode::Space* getGecodeModel(void);

    // helpers for getting correct int bounds
    bool valueWithinBounds(double b);

    // helper functions for processing flatzinc constraints
    /// Convert \a arg (array of integers) to IntArgs
    Gecode::IntArgs arg2intargs(Expression* arg, int offset = 0);
    /// Convert \a arg (array of Booleans) to IntArgs
    Gecode::IntArgs arg2boolargs(Expression* arg, int offset = 0);
    /// Convert \a n to IntSet
    Gecode::IntSet arg2intset(EnvI& envi, Expression* sl);
    /// Convert \a arg to IntVarArgs   
    Gecode::IntVarArgs arg2intvarargs(FznSpace* space, Expression* arg, int offset = 0); 
    /// Convert \a arg to BoolVarArgs
    Gecode::BoolVarArgs arg2boolvarargs(FznSpace* space, Expression* a, int offset = 0, int siv=-1);
    /// Convert \a n to BoolVar
    Gecode::BoolVar arg2boolvar(FznSpace* space, Expression* e);
    /// Convert \a n to IntVar
    Gecode::IntVar arg2intvar(FznSpace* space, Expression* e);
     /// convert \a arg to an ArrayLit (throws InternalError if not possible)
    ArrayLit* arg2arraylit(Expression* arg);  
    /// Check if \a b is array of Booleans (or has a single integer)
    bool isBoolArray(ArrayLit* a, int& singleInt);
#ifdef GECODE_HAS_FLOAT_VARS
    /// Convert \a n to FloatValArgs
    Gecode::FloatValArgs arg2floatargs(Expression* arg, int offset = 0);
    /// Convert \a n to FloatVar
    Gecode::FloatVar arg2floatvar(FznSpace* space, Expression* n);
    /// Convert \a n to FloatVarArgs
    Gecode::FloatVarArgs arg2floatvarargs(FznSpace* space, Expression* arg, int offset = 0);
#endif
    /// Convert \a ann to IntConLevel
    Gecode::IntConLevel ann2icl(const Annotation& ann);
     /// convert the annotation \a s int variable selection to the respective Gecode var selection
    Gecode::TieBreak<Gecode::IntVarBranch> ann2ivarsel(std::string s, Gecode::Rnd& rnd, double decay);
    /// convert the annotation \a s int value selection to the respective Gecode val selection
    Gecode::IntValBranch ann2ivalsel(std::string s, std::string& r0, std::string& r1, Gecode::Rnd& rnd);
    /// convert assign value selection
    Gecode::IntAssign ann2asnivalsel(std::string s, Gecode::Rnd& rnd);
#ifdef GECODE_HAS_SET_VARS
    Gecode::SetVarBranch ann2svarsel(std::string s, Gecode::Rnd& rnd, double decay);
    Gecode::SetValBranch ann2svalsel(std::string s, std::string r0, std::string r1, Gecode::Rnd& rnd);
#endif
#ifdef GECODE_HAS_FLOAT_VARS
    Gecode::TieBreak<Gecode::FloatVarBranch> ann2fvarsel(std::string s, Gecode::Rnd& rnd, double decay);
    Gecode::FloatValBranch ann2fvalsel(std::string s, std::string r0, std::string r1);
#endif    
    /// Returns the VarDecl of \a expr and throws an InternalError if not possible
    VarDecl* getVarDecl(Expression* expr);
    /// Returns the VarDecl of \a aa 
    VarDecl* resolveArrayAccess(ArrayAccess* aa);
    /// Returns the VarDecl of \a array at index \a index
    VarDecl* resolveArrayAccess(VarDecl* array, int index);

    /// Returns the GecodeVariable representing the Id, VarDecl or ArrayAccess
    GecodeSolver::Variable resolveVar(Expression* e);     

    /// Inserts variable gv into _variableMap with key id
    void insertVar(Id* id, GecodeVariable gv);    

    void assignSolutionToOutput(void);   

  protected:
    /// Flatzinc options // TODO: do we need specific Gecode options? Use MiniZinc::Options instead?
    // FlatZincOptions* opts;
    void registerConstraints(void);
    void registerConstraint(std::string name, poster p);

    /// creates the gecode branchers // TODO: what is decay, ignoreUnknown -> do we need all the args?
    void createBranchers(Annotation& ann, Expression* additionalAnn, int seed, double decay,
            bool ignoreUnknown, std::ostream& err);
    /// prepare the engine, where combinators states if search combinators will be used; optimize combinator is true only if combinators are used and we are optimizing (BAB)
    void prepareEngine(bool combinators, bool optimize_combinator = false);  
    /// sets the search strategy according to the search annotation
    void setSearchStrategyFromAnnotation(std::vector<Expression*> flatAnn);
    /// Helper: will retrieve an ArrayLit from flatzinc, if it is par, or is var with a right hand side, otherwise throws exception
    ArrayLit* getArrayLit(Expression* e);  
    /// set the search strategy according to the search annotation
    void setSearchStrategyFromAnnotation(std::vector<Expression*> flatAnn, 
                                                        std::vector<bool>& iv_searched, 
                                                        std::vector<bool>& bv_searched,
                                                        std::vector<bool>& sv_searched,
                                                        std::vector<bool>& fv_searched,
                                                        Gecode::TieBreak<Gecode::IntVarBranch>& def_int_varsel,
                                                        Gecode::IntValBranch& def_int_valsel,
                                                        Gecode::TieBreak<Gecode::IntVarBranch>& def_bool_varsel,
                                                        Gecode::IntValBranch& def_bool_valsel,
                                                #ifdef GECODE_HAS_SET_VARS
                                                        Gecode::SetVarBranch& def_set_varsel,
                                                        Gecode::SetValBranch& def_set_valsel,
                                                #endif
                                                #ifdef GECODE_HAS_FLOAT_VARS
                                                        Gecode::TieBreak<Gecode::FloatVarBranch>& def_float_varsel,
                                                        Gecode::FloatValBranch& def_float_valsel,
                                                #endif
                                                        Gecode::Rnd& rnd,
                                                        double decay,
                                                        bool ignoreUnknown,
                                                        std::ostream& err
                                                       );
  };
}

#endif
