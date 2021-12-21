#include "dead-function-analyzer-util.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/ValueHandle.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include <algorithm>
#include <map>
#include <set>
#include <stack>
#include <vector>
#include <llvm/ADT/DepthFirstIterator.h>
#include "llvm/ADT/GraphTraits.h"

using namespace llvm;
using namespace std;

struct DeadFuncOptimizer : public ModulePass {
  static char ID; // Pass identification, replacement for typeid
  DeadFuncOptimizer() : ModulePass(ID) {}

  /*
   * We are assuming that the "main" function is the entry point of a module.
   * This function extracts the function pointer from the function named "main."
   */
  Function *extractEntryFunction(vector<Function *> &allFunctions) {
    for (auto function : allFunctions) {
      if (function->getName().str() == "main") {
        return function;
      }
    }
    return nullptr;
  }

  /*
   * Function for determining call graphs. Given a list of all functions, we iterate over functions, extract callee from CallInst, then add edge 
   */
  map<Function *, vector<Function *>>
  getCallGraph(vector<Function *> &allFunctions) {
    map<Function *, vector<Function *>> callGraph;
    for (Function *f : allFunctions) {
      vector<Function *> callee;
      for (BasicBlock &basicBlocks : *f) {
        for (Instruction &instruction : basicBlocks) {
          Instruction *instPtr = &instruction;
          if (isa<CallInst>(instPtr)) {
            CallInst *callInst = dyn_cast<CallInst>(instPtr);
            callee.push_back(callInst->getCalledFunction());
          }
        }
      }
      callGraph[f] = callee;
    }
    return callGraph;
  }


  vector<Function *> getDeadFunctions(vector<Function *> &allFunctions,
                    map<Function *, vector<Function *>> &callGraph,
                    Function *entryFunction) {
    vector<Function *> dead;
    bool included;
    //how do we adjust the dfs to only check functions reachable from the entry function
    for (auto theFunction: allFunctions){
        included = false;
        for ( CallGraph::const_iterator itr = callGraph.begin(), ie = callGraph.end() ; itr != ie; itr++)
    {
    //must check if function is contained within the call graph
            if (theFunction == itr->getFunction())
            {
                included = true;
            }
        if (included == false)
        {
            dead.push_back(theFunction);
        }}}
    /*
     * TODO: extract the dead functions. If a function is unreachable from
     *       the entryFunction, that function will be deemed dead. You have 
     *       to extract all such unused functions and put those in the 
     *       vector `dead`.
     */
    return dead;}

  void removeDeadFunctions(Module &M, vector<Function *> &deadFunctions) {

        for (auto itr: deadFunctions)
        {
            itr->replaceAllUsesWith(UndefValue::get(itr->getType);
            itr->eraseFromParent();
        }
  }
    /*
     * TODO: remove all the dead functions from Module M.
     *       Remember, dead functions are functions that cannot be reached from
     *       the entryFunction. That doesn't mean that other functions cannot call
     *       such functions. If you don't take necessary steps before removing such 
     *       functions, your optimizer might crash. TAs will not provide any further 
     *       hints about what necessary steps you should take or how to remove these
     *       functions, especially since this is extra credit. You should do your own
     *       research on necessary APIs for implementing this function.
     */


  bool runOnModule(Module &M) override {

    OptimizationResultWriter writer(M);
    vector<Function *> allFunctions;

    for (Function &F : M) {
      allFunctions.push_back(&F);
    }
    map<bx *, vector<Function *>> callGraph = getCallGraph(allFunctions);
    writer.printCallGraph(callGraph);
    Function *entryFunction = extractEntryFunction(allFunctions);
    if (nullptr != entryFunction ) {
      vector<Function *> deadFunctions =
          getDeadFunctions(allFunctions, callGraph, entryFunction);
      writer.printDeadFunctions(deadFunctions);
      removeDeadFunctions(M, deadFunctions);
      writer.writeModifiedModule(M);
    }
    return true;
  }
};

char DeadFuncOptimizer::ID = 0;
static RegisterPass<DeadFuncOptimizer> X("optimize", "Optimization Pass");
