#include <iostream>
#include <stdlib.h>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/PassManager.h"
#include "llvm/Assembly/PrintModulePass.h"
#include "llvm/IR/User.h"
#include "clang/Driver/Types.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InstVisitor.h"
#include "DDG.h"



using namespace llvm;
using namespace std;

struct CountAllocaVisitor : public InstVisitor<CountAllocaVisitor> {
    unsigned Count;
    CountAllocaVisitor() : Count(0) {}
    
    //void visitAllocaInst(AllocaInst &AI) { ++Count; }
    void visitLoadInst(LoadInst &LI) { ++Count; }
};

// Performance Prediction Pass
class CanyonPass : public FunctionPass {
public:
	static char ID;
    CanyonPass() : FunctionPass(ID) {}

    void getAnalysisUsage(AnalysisUsage &AU) const {
    	AU.addRequired<DependenceAnalysis>();
        AU.addRequired<LoopInfo>();
    }

    bool runOnFunction(Function &F) override {
        DependenceAnalysis *DA = &(getAnalysis<DependenceAnalysis>());
        LoopInfo &LI = getAnalysis<LoopInfo>();
        
        CountAllocaVisitor CAV;
        CAV.visit(F);
        int NumAllocas = CAV.Count;
        errs() << "The number of malloc insts is: " << NumAllocas << "\n";
        
        // Construct DDG        
        //DataDepGraph *ddg = new DataDepGraph(&F, DA);

        // iterate over loops
        for (LoopInfo::iterator L = LI.begin(), LE = LI.end(); L != LE; ++L) {
            llvm::Loop *loop = *L;
            errs() << *loop << "\n";
            unsigned loop_depth = loop->getLoopDepth();
            errs() << "loop_depth = " << loop_depth << "\n";
            // iterate over basic blocks
            unsigned num_bb = 0;
            for (LoopBase<BasicBlock, Loop>::block_iterator B = loop->block_begin(), BE = loop->block_end(); B != BE; ++B) {
                ++num_bb;
                errs() << "  BB-" << num_bb << ":\n";
                // iterate over instructions
                unsigned num_inst = 0;
                for (BasicBlock::iterator I = (*B)->begin(), IE = (*B)->end(); I != IE; ++I) {
                    ++num_inst;
                    errs() << "    Inst-" << num_inst << ":" << *I << "\n";
                }
            }
            //errs() << (**L) << "\n";

        }
        return false;
    }
};

char CanyonPass::ID = 0;
static RegisterPass<CanyonPass> X("canyon", "Performance Prediction Pass", false, false);
