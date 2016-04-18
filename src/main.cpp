#include <iostream>
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
#include "DDG.h"


using namespace llvm;
using namespace std;

int main(int argc, char **argv) {
    if (argc < 2) {
      errs() << "Usage: " << argv[0] << " <IR file>\n";
      return 1;
    }

    // read LLVM IR file and save it as a module.
    SMDiagnostic Err;
    unique_ptr<Module> Mod(ParseIRFile(argv[1], Err, getGlobalContext()));
    if (!Mod) {
        Err.print(argv[0], errs());
        return 1;
    }
  
    //PassManager PM;
    //PM.add(createPrintModulePass(&outs()));
    //PM.run(*Mod);


    Module::FunctionListType &func_list = Mod->getFunctionList();
  
    // construct ddg for each function in Mod
    map<Function*,DataDepGraph*> ddg_map; 
    for (llvm::Module::FunctionListType::iterator func = func_list.begin(),func_list_end = func_list.end(); func != func_list_end; ++func) {
        DataDepGraph *ddg = new DataDepGraph(func);
        pair<Function*,DataDepGraph*> func_ddg_pair (func,ddg);
        ddg_map.insert(func_ddg_pair);
    }

    errs() << "WS\n";
    return 0;
}
