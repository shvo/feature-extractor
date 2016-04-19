#ifndef PROFILER_H
#define PROFILER_H

#include "llvm/IR/Instructions.h"
#include "llvm/InstVisitor.h"

using namespace llvm;

struct CountFeatureInstNum : public InstVisitor<CountFeatureInstNum> {
    // declare counters
    unsigned Store_Count;
    unsigned Load_Count;
    unsigned GetElementPtr_Count;

    // initialized counters
    CountFeatureInstNum() : Store_Count(0), Load_Count(0), GetElementPtr_Count(0) {}
    
    // define counters
    void visitStoreInst(StoreInst &SI) { ++Store_Count; }
    void visitLoadInst(LoadInst &LI) { ++Load_Count; }
    void visitGetElementPtrInst(GetElementPtrInst &GEPI) { ++GetElementPtr_Count; }
};

#endif
