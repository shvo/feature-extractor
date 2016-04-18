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



using namespace llvm;

class TestPass : public FunctionPass {
public:
	static char ID;
    TestPass() : FunctionPass(ID) {}

    void getAnalysisUsage(AnalysisUsage &AU) const {
    	AU.addRequired<AliasAnalysis>();
    	AU.addRequired<DependenceAnalysis>();
    }

    typedef SmallVector<Value *, 16> ValueVector;
    ValueVector MemInstr;

    bool runOnFunction(Function &F) override {
    	AliasAnalysis AA = getAnalysis<AliasAnalysis>();
        DependenceAnalysis *DA = &(getAnalysis<DependenceAnalysis>());
        // iterate over basic blocks
        Function *func = &F;
        unsigned bb_num = 0;
        for (Function::iterator BB = func->begin(), BE = func->end();
       BB != BE; ++BB) {
        	errs() << "BB-" << bb_num << "\n";
            bb_num++;
            // iterator over instructions
            unsigned inst_num = 0;
            for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E;++I) {
                Instruction *Ins = dyn_cast<Instruction>(I);
                if (!Ins)
                    return false;
                LoadInst *Ld = dyn_cast<LoadInst>(I);
                StoreInst *St = dyn_cast<StoreInst>(I);
                if (!St && !Ld)
                    continue;
                if (Ld && !Ld->isSimple())
                    return false;
                if (St && !St->isSimple())
                    return false;
                inst_num++;
                MemInstr.push_back(&*I);
                errs() << "MemInst-" << inst_num << ":" << *I << "\n";
            }

        	ValueVector::iterator I, IE, J, JE;
            for (I = MemInstr.begin(), IE = MemInstr.end(); I != IE; ++I) {
            for (J = I, JE = MemInstr.end(); J != JE; ++J) {
                std::vector<char> Dep;
                Instruction *Src = dyn_cast<Instruction>(*I);
                Instruction *Des = dyn_cast<Instruction>(*J);
                if (Src == Des)
                    continue;
                if (isa<LoadInst>(Src) && isa<LoadInst>(Des))
                    continue;
                if (auto D = DA->depends(Src, Des, true)) {
                    errs() << "Found Dependency between:\nSrc:" << *Src << "\nDes:" << *Des
                     << "\n";
                	if (D->isFlow()) {
                    	errs () << "Flow dependence not handled";
                    	return false;
                	}
                
                	if (D->isAnti()) {
                    	errs() << "Found Anti dependence \n";

						AliasAnalysis::AliasResult AA_dep = AA.alias(Src, Des);
                        AliasAnalysis::AliasResult AA_dep_1 = AA.alias(Des, Src);
                    	errs() << "The Ld->St alias result is " << AA_dep << "\n";
                    	errs() << "The St->Ld alias result is " << AA_dep_1 << "\n";
                    	
                        unsigned Levels = D->getLevels();
                    	errs() << "levels = " << Levels << "\n";
                    	char Direction;
                    	for (unsigned II = 1; II <= Levels; ++II) {
                        	const SCEV *Distance = D->getDistance(II);
                        	const SCEVConstant *SCEVConst = dyn_cast_or_null<SCEVConstant>(Distance);
                        	if (SCEVConst) {
                            	const ConstantInt *CI = SCEVConst->getValue();
                                //int64_t it_dist = CI->getUniqueInteger().getSExtValue();
                                //int it_dist = CI->getUniqueInteger().getSExtValue();
                               	unsigned it_dist = abs(CI->getUniqueInteger().getSExtValue());
                            	errs() << "distance is not null\n";
                            	//errs() << "distance = "<< *CI << "\n";
                            	errs() << "distance = "<< it_dist << "\n";
                       			if (CI->isNegative())
                            		Direction = '<';
                        		else if (CI->isZero())
                            		Direction = '=';
                        		else
                            		Direction = '>';
                        		Dep.push_back(Direction);
                        	} 
                        	else if (D->isScalar(II)) {
                        		Direction = 'S';
                        		Dep.push_back(Direction);
                        	} 
                        	else {
                            	unsigned Dir = D->getDirection(II);
                            	if (Dir == Dependence::DVEntry::LT || Dir == Dependence::DVEntry::LE)
                                	Direction = '<';
                            	else if (Dir == Dependence::DVEntry::GT || Dir == Dependence::DVEntry::GE)
                                	Direction = '>';
                            	else if (Dir == Dependence::DVEntry::EQ)
                                	Direction = '=';
                            	else
                                	Direction = '*';
                            	Dep.push_back(Direction);
                        	}
                    	}
                	}
              	}
            }
        	}
        }
        errs() << "------Hello World!--------\n";
        return false;
    }
};

char TestPass::ID = 0;
static RegisterPass<TestPass> X("test", "-----WANG SHUO----", false, false);
