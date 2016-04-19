#ifndef PROFILER_H
#define PROFILER_H

#include "llvm/IR/User.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InstVisitor.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/Type.h"

using namespace llvm;

struct CountFeatureInstNum : public InstVisitor<CountFeatureInstNum> {
    // declare counters
    unsigned Store_Count;
    unsigned Load_Count;
    unsigned GetElementPtr_Count;
    unsigned FP_Count;
    unsigned INT_Count;
    unsigned Branch_Count;

    // initialized counters
    CountFeatureInstNum() : Store_Count(0), Load_Count(0), GetElementPtr_Count(0) 
                           ,FP_Count(0), INT_Count(0), Branch_Count(0) {}
    
    // define counters
    void visitStoreInst(StoreInst &SI) { ++Store_Count; }
    void visitLoadInst(LoadInst &LI) { ++Load_Count; }
    void visitGetElementPtrInst(GetElementPtrInst &GEPI) { ++GetElementPtr_Count; }
    void visitBinaryOperator(BinaryOperator &BOI) { 
        Value *opd = BOI.getOperand(0);
        Type *opd_type = opd->getType();
        /*
        errs() << "Inst: " << BOI << "\n";
        errs() << "operand: " << *opd << "\n";
        errs() << "type: " << *opd_type << "\n";
        */
        if (opd_type->isFloatingPointTy()) {
            ++FP_Count; 
        } else {
            ++INT_Count;
        }
    }
    void visitBranchInst(BranchInst &BI) { ++Branch_Count;}
};

#endif
