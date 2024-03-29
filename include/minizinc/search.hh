/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */

/*
 *  Main authors:
 *     Andrea Rendl <andrea.rendl@nicta.com.au>
 */

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __MINIZINC_SEARCH_HH__
#define __MINIZINC_SEARCH_HH__

#include <minizinc/flatten.hh>
#include <minizinc/solver_instance_base.hh>
#include <minizinc/flatten_internal.hh>

namespace MiniZinc {
  
  class SearchHandler { 
  private:
  protected:
    // the stack of scopes where the first scope is the root scope
    std::vector<SolverInstanceBase*> _scopes;   
    // list of timeouts; the most recently set timeout is the last in the list
    std::vector<long long int> _timeouts;
    // the index of the timeout (in the timeout list) that has been reached
    int _timeoutIndex;
    // stack of flags for breaking out of repeat loops
    std::vector<bool> _repeat_break;
    // the number of intermediate variables that we have added locally in the scope (last variable contains current scope)
    std::vector<int> _localVarsToAdd;
    // the list of variables (per scope) that are added locally
    std::vector<std::vector<VarDecl*> > _localVars;
  public:
    SearchHandler() : _timeoutIndex(-1) {}
    
    /// perform search on the flat model in the environement using the specified solver
    template<class SolverInstanceBase>
    void search(Env& env, MiniZinc::Options& opt) {
      SolverInstanceBase* solver = new SolverInstanceBase(env,opt);     
      solver->processFlatZinc();
      
      bool verbose = opt.getBoolParam(constants().opts.verbose.str(),false);
      
      SolverInstance::Status status;    
      Expression* combinator = NULL;
      if(env.flat()->solveItem()->combinator_lite()) {        
        Annotation& ann = env.flat()->solveItem()->ann(); 
        for(ExpressionSetIter it = ann.begin(); it!=ann.end(); it++) {
          if(Call* c = (*it)->dyn_cast<Call>()) {
            if(c->id() == constants().ann.combinator) {
              combinator = c->args()[0];
              break;
            }
          }  
        }
        ann.removeCall(constants().ann.combinator);
        pushScope(solver);
        combinator = removeRedundantScopeCombinator(combinator,solver,verbose);
        env.envi().pushSolution(NULL);
        status = interpretCombinator(combinator, solver, verbose);        
        env.envi().popSolution();
        popScope();
      }
      else { // solve using normal solve call
        status = solver->solve();
        env.evalOutput(std::cout); // print solution
      }    
      switch(status) {
        case SolverInstance::SUCCESS:
          std::cout << constants().solver_output.sat << std::endl;
          break;
        case SolverInstance::FAILURE:
          std::cout << constants().solver_output.unsat << std::endl;
          break;        
      }
    }
    
  private:
    /// interpret and execute the given combinator  
  SolverInstance::Status interpretCombinator(Expression* comb, SolverInstanceBase* solver, bool verbose);
  /// interpret and execute an AND combinator
  SolverInstance::Status interpretAndCombinator(Call* andComb, SolverInstanceBase* solver, bool verbose);
  /// interpret and execute MAXIMISE/MINIMIZE combinator (depending on minimize flag)
  SolverInstance::Status interpretBestCombinator(Call* c, SolverInstanceBase* solver, bool minimize, bool print, bool verbose);
  /// interpret and execute an operator given as BinOp expression (AND as well as OR)
  SolverInstance::Status interpretBinOpCombinator(BinOp* bo, SolverInstanceBase* solver, bool verbose);
  /// interpret if-then-else combibator
  SolverInstance::Status interpretConditionalCombinator(Expression* call, SolverInstanceBase* solver, bool verbose);
  /// interpret and execute a new scope (defined by let); isNested is false if the LET combinator is the top-most combinator and false otherwise  
  SolverInstance::Status interpretLetCombinator(Let* let, SolverInstanceBase* solver, bool verbose);
   /// interpret and execute an OR combinator
  SolverInstance::Status interpretOrCombinator(Call* orComb, SolverInstanceBase* solver, bool verbose);
   /// interpret and execute a POST combinator
  SolverInstance::Status interpretPostCombinator(Call* postComb, SolverInstanceBase* solver, bool verbose);
  /// interpret and execute a REPEAT combinator
  SolverInstance::Status interpretRepeatCombinator(Call* repeatComb, SolverInstanceBase* solver, bool verbose);
    /// interpret and execute a SCOPE combinator
  SolverInstance::Status interpretScopeCombinator(Call* scopeComb, SolverInstanceBase* solver, bool verbose);
   /// interpret and execute a NEXT combinator
  SolverInstance::Status interpretNextCombinator(SolverInstanceBase* solver, bool verbose);
  /// interpret and execute a NEXT combinator, respecting the arguments given in the call
  SolverInstance::Status interpretNextCombinator(Call* call, SolverInstanceBase* solver, bool verbose);
  /// interpret and execute a PRINT combinator as call (possibly with arguments)
  SolverInstance::Status interpretPrintCombinator(Call* call, SolverInstanceBase* solver, bool verbose);
  /// interpret and execute a PRINT combinator as Id
  SolverInstance::Status interpretPrintCombinator(SolverInstanceBase* solver, bool verbose);
  /// interpret and execute a comb_assign combinator
  SolverInstance::Status interpretAssignCombinator(Call* assignComb, SolverInstanceBase* solver, bool verbose);
  /// interpret and execute a SCOPE combinator
  SolverInstance::Status interpretTimeLimitAdvancedCombinator(Call* scopeComb, SolverInstanceBase* solver, bool verbose);
  /// interpret and execute a COMMIT combinator
  SolverInstance::Status interpretCommitCombinator(Call* commitComb, SolverInstanceBase* solver, bool verbose);
  /// interpret and execute a BREAK combinator
  SolverInstance::Status interpretBreakCombinator(Id* c, SolverInstanceBase* solver, bool verbose);
  /// post the list of (unflattened) constraints (the argument of the POST combinator) in the solver
  bool postConstraints(Expression* cts, SolverInstanceBase* solver, bool verbose);
  /// overwrite the solution in \a outputToUpdate with the solution in \a output
  void updateSolution(Model* output, Model* outputToUpdate); 
  /// interpret the LIMIT combinator \a e which is either a Call or an array of Calls
  void interpretLimitCombinator(Expression* e, SolverInstanceBase* solver, bool verbose);
  /// process node limit combinator
  void interpretFailLimitCombinator(Call* call, SolverInstanceBase* solver, bool verbose);
  /// process node limit combinator
  void interpretNodeLimitCombinator(Call* call, SolverInstanceBase* solver, bool verbose);
  /// process solution limit combinator
  void interpretSolutionLimitCombinator(Call* call, SolverInstanceBase* solver, bool verbose);
  /// process a time limit combinator
  void interpretTimeLimitCombinator(Call* call, SolverInstanceBase* solver, bool verbose);
  /// if the search combinator call has an initial SCOPE combinator, remove it, because it can be ignored
  Expression* removeRedundantScopeCombinator(Expression* combinator, SolverInstanceBase* solver, bool verbose);
  /// add the new variable (defined by a LET) to the model, and add it to the output model so we can retrieve solutions of it
  void addNewVariableToModel(ASTExprVec<Expression> decls, SolverInstanceBase* solver, bool verbose);
  
  void pushScope(SolverInstanceBase* new_scope) {    
    _scopes.push_back(new_scope);   
    _localVarsToAdd.push_back(0);
  }
  void popScope() {
    delete _scopes.back();
    _scopes.pop_back();
    _localVarsToAdd.pop_back();
  }
  /// returns true if a timelimit is violated and false otherwise
  bool isTimeLimitViolated(bool verbose = false);
  /// returns the index of the time-limit if any of the current time limits is violated, and -1 otherwise
  int getViolatedTimeLimitIndex(bool verbose = false);
  /// set the index of the currently violated time limit
  void setTimeoutIndex(int index);
  // reset the timeout index
  void resetTimeoutIndex(void);
  /// returns the timeout time: time-now + timeout(given-in-milliseconds)
  //long long int getTimeout(int ms);
  /// sets timeout options (for next) in case there is a timeout
  void setCurrentTimeout(SolverInstanceBase* solver);
  };
 
  
}

#endif
