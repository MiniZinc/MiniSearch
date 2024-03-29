/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */

/*
 *  Main authors:
 *     Guido Tack <guido.tack@monash.edu>
 */

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __MINIZINC_MODEL_HH__
#define __MINIZINC_MODEL_HH__

#include <vector>
#include <iterator>

#include <minizinc/gc.hh>
#include <minizinc/ast.hh>
#include <minizinc/hash.hh>
#include <minizinc/prettyprinter.hh>

namespace MiniZinc {
  
  class VarDeclIterator;
  class ConstraintIterator;
  
  class CopyMap;
  class EnvI;
  
  /// A MiniZinc model
  class Model {
    friend class GC;
    friend Model* copy(EnvI& env, CopyMap& cm, Model* m, bool followIncludes, bool isFlatModel);

  protected:
    /// Previous model in root set list
    Model* _roots_prev;
    /// Next model in root set list
    Model* _roots_next;

    /// Type of map from identifiers to function declarations
    typedef ASTStringMap<std::vector<FunctionI*> >::t FnMap;
    /// Map from identifiers to function declarations
    FnMap fnmap;

    /// Filename of the model
    ASTString _filename;
    /// Path of the model
    ASTString _filepath;
    /// Parent model if model was included
    Model* _parent;
    /// Items in the model
    std::vector<Item*> _items;
    /// Pointer to the solve item
    SolveI* _solveItem;
    /// Pointer to the output item
    OutputI* _outputItem;
    /// File-level documentation comment
    std::string _docComment;
    /// Flag whether model is failed
    bool _failed;
  public:
    
    /// Construct empty model
    Model(void);
    /// Destructor
    ~Model(void);
    
    /// Add \a i to the model
    void addItem(Item* i) {
      _items.push_back(i);
      if (i->isa<SolveI>()) {
        _solveItem = i->cast<SolveI>();        
      } else if (i->isa<OutputI>()) {
        _outputItem = i->cast<OutputI>();
      }
    }
    
    /// Get parent model
    Model* parent(void) const { return _parent; }
    /// Set parent model to \a p
    void setParent(Model* p) { assert(_parent==NULL); _parent = p; }
    
    /// Get file name
    ASTString filename(void) const { return _filename; }
    /// Get file path
    ASTString filepath(void) const { return _filepath; }
    /// Set file name
    void setFilename(const std::string& f) {
      assert(_filename.size()==0);
      _filename = ASTString(f);
    }
    /// Set file path
    void setFilepath(const std::string& f) {
      assert(_filepath.size()==0);
      _filepath = ASTString(f);
    }

    /// Register a builtin function item
    void registerFn(EnvI& env, FunctionI* fi);
    /// Sort functions by type
    void sortFn(void);
    /// Return function declaration for \a id matching \a args
    FunctionI* matchFn(EnvI& env, const ASTString& id,
                       const std::vector<Expression*>& args) const;
    /// Return function declaration for \a id matching types \a t
    FunctionI* matchFn(EnvI& env, const ASTString& id, const std::vector<Type>& t);
    /// Return function declaration matching call \a c
    FunctionI* matchFn(EnvI& env, Call* c) const;

    /// Return item \a i
    Item*& operator[] (int i);
    /// Return item \a i
    const Item* operator[] (int i) const;
    /// Return number of items
    unsigned int size(void) const;
    
    typedef std::vector<Item*>::iterator iterator;
    typedef std::vector<Item*>::const_iterator const_iterator;
    
    /// Iterator for beginning of items
    iterator begin(void);
    /// Iterator for beginning of items
    const_iterator begin(void) const;
    /// Iterator for end of items
    iterator end(void);
    /// Iterator for end of items
    const_iterator end(void) const;

    ConstraintIterator begin_constraints(void);
    ConstraintIterator end_constraints(void);
    VarDeclIterator begin_vardecls(void);
    VarDeclIterator end_vardecls(void);
    
    SolveI* solveItem(void);

    OutputI* outputItem(void);

    
    /// Add a file-level documentation comment
    void addDocComment(std::string s) { _docComment += s; }

    /// Return the file-level documentation comment
    const std::string& docComment(void) const { return _docComment; }
    
    /// Remove all items marked as removed
    void compact(void);
    
    /// Make model failed
    void fail(EnvI& env);

    /// Return whether model is known to be failed
    bool failed(void) const;

    Model::FnMap::iterator fnmap_begin(void) { return fnmap.begin(); }
    Model::FnMap::iterator fnmap_end(void) { return fnmap.end(); }
  };

  class VarDeclIterator {
    Model* _model;
    Model::iterator _it;
  public:
    typedef Model::iterator::difference_type difference_type;
    typedef Model::iterator::value_type value_type;
    typedef VarDeclI& reference;
    typedef VarDeclI* pointer;
    typedef std::forward_iterator_tag iterator_category;
    
    VarDeclIterator() {}
    VarDeclIterator(const VarDeclIterator& vi) : _it(vi._it) {}
    VarDeclIterator(Model* model, const Model::iterator& it) : _model(model), _it(it) {
      while (_it != _model->end() && (!(*_it)->isa<VarDeclI>() || (*_it)->removed())) {
        ++_it;
      }
    }
    ~VarDeclIterator() {}
    
    VarDeclIterator& operator=(const VarDeclIterator& vi) {
      if (this != &vi) {
        _it = vi._it;
      }
      return *this;
    }
    bool operator==(const VarDeclIterator& vi) const { return _it == vi._it; }
    bool operator!=(const VarDeclIterator& vi) const { return _it != vi._it; }
    VarDeclIterator& operator++() {
      do {
        ++_it;
      } while (_it != _model->end() && (!(*_it)->isa<VarDeclI>() || (*_it)->removed()));
      return *this;
    }
    
    reference operator*() const { return *(*_it)->cast<VarDeclI>(); }
    pointer operator->() const { return (*_it)->cast<VarDeclI>(); }
  };

  class ConstraintIterator {
    Model* _model;
    Model::iterator _it;
  public:
    typedef Model::iterator::difference_type difference_type;
    typedef Model::iterator::value_type value_type;
    typedef ConstraintI& reference;
    typedef ConstraintI* pointer;
    typedef std::forward_iterator_tag iterator_category;
    
    ConstraintIterator() {}
    ConstraintIterator(const ConstraintIterator& vi) : _it(vi._it) {}
    ConstraintIterator(Model* model, const Model::iterator& it) : _model(model), _it(it) {
      while (_it != _model->end() && !(*_it)->isa<ConstraintI>()) {
        ++_it;
      }
    }
    ~ConstraintIterator() {}
    
    ConstraintIterator& operator=(const ConstraintIterator& vi) {
      if (this != &vi) {
        _it = vi._it;
      }
      return *this;
    }
    bool operator==(const ConstraintIterator& vi) const { return _it == vi._it; }
    bool operator!=(const ConstraintIterator& vi) const { return _it != vi._it; }
    ConstraintIterator& operator++() {
      do {
        ++_it;
      } while (_it != _model->end() && !(*_it)->isa<ConstraintI>());
      return *this;
    }
    
    reference operator*() const { return *(*_it)->cast<ConstraintI>(); }
    pointer operator->() const { return (*_it)->cast<ConstraintI>(); }
  };

  
  class EnvI;
  class CopyMap;
  class FlatteningOptions;
  
  /// Environment
  class Env {
  private:
    EnvI* e;    
    Env(Model* orig, Model* output, Model* flat, CopyMap& cmap, IdMap<KeepAlive> reverseMappers,
    unsigned int ids, const FlatteningOptions& fopts);
  public:
    Env(Model* m);
    Env(Model* m, const FlatteningOptions& fopt);
    ~Env(void);
    
    Model* model(void);
    Model* flat(void);
    void swap();
    Model* output(void);
    EnvI& envi(void);
    const EnvI& envi(void) const;
    std::ostream& dumpErrorStack(std::ostream& os);
    const std::vector<std::string>& warnings(void);
    void clearWarnings(void);
    unsigned int maxCallStack(void) const;
    std::ostream& evalOutput(std::ostream& os);
    Expression* combinator;
    Env* copyEnv(CopyMap& cmap);    
  };

  class CallStackItem {
  public:
    EnvI& env;
    CallStackItem(EnvI& env0, Expression* e);
    CallStackItem(EnvI& env0, Id* ident, IntVal i);
    ~CallStackItem(void);
  };

  /// Visitor for model items
  class ItemVisitor {
  public:
    /// Enter model
    bool enterModel(Model* m) { return true; }
    /// Enter item
    bool enter(Item* m) { return true; }
    /// Visit include item
    void vIncludeI(IncludeI*) {}
    /// Visit variable declaration
    void vVarDeclI(VarDeclI*) {}
    /// Visit assign item
    void vAssignI(AssignI*) {}
    /// Visit constraint item
    void vConstraintI(ConstraintI*) {}
    /// Visit solve item
    void vSolveI(SolveI*) {}
    /// Visit output item
    void vOutputI(OutputI*) {}
    /// Visit function item
    void vFunctionI(FunctionI*) {}
  };

  /// Iterator over items in a model and all its included models
  template<class I>
  class ItemIter {
  protected:
    I& iter;
  public:
    ItemIter(I& iter0) : iter(iter0) {}
    void run(Model* m) {
      UNORDERED_NAMESPACE::unordered_set<Model*> seen;
      std::vector<Model*> models;
      models.push_back(m);
      seen.insert(m);
      while (!models.empty()) {
        Model* cm = models.back();
        models.pop_back();
        if (!iter.enterModel(cm))
          continue;
        for (unsigned int i=0; i<cm->size(); i++) {
          if ((*cm)[i]->removed())
            continue;
          if (!iter.enter((*cm)[i]))
            continue;
          switch ((*cm)[i]->iid()) {
          case Item::II_INC:
            if (seen.find((*cm)[i]->cast<IncludeI>()->m()) == seen.end()) {
              models.push_back((*cm)[i]->cast<IncludeI>()->m());
              seen.insert((*cm)[i]->cast<IncludeI>()->m());
            }
            iter.vIncludeI((*cm)[i]->cast<IncludeI>());
            break;
          case Item::II_VD:
            iter.vVarDeclI((*cm)[i]->cast<VarDeclI>());
            break;
          case Item::II_ASN:
            iter.vAssignI((*cm)[i]->cast<AssignI>());
            break;
          case Item::II_CON:
            iter.vConstraintI((*cm)[i]->cast<ConstraintI>());
            break;
          case Item::II_SOL:
            iter.vSolveI((*cm)[i]->cast<SolveI>());
            break;
          case Item::II_OUT:
            iter.vOutputI((*cm)[i]->cast<OutputI>());
            break;
          case Item::II_FUN:
            iter.vFunctionI((*cm)[i]->cast<FunctionI>());
            break;      
          }
        }
      }
    }
  };
  
  /// Run iterator \a i over all items of model \a m
  template<class I>
  void iterItems(I& i, Model* m) {
    ItemIter<I>(i).run(m);
  }
  
}

#endif
