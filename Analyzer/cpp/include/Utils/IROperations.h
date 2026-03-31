#ifndef _IR_OPERATION_H_
#define _IR_OPERATION_H_

#include <vector>

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/DebugInfo.h" 

struct ValPathStep {
	llvm::Value *val;
	int offset;
};

std::vector<ValPathStep> getValPath(llvm::Value *val);
std::pair<llvm::Value *, std::vector<int> > getOffsetValPath(llvm::Value *val);

llvm::DIVariable *getDbgVar(llvm::Value *val);
std::string getSourcePath(llvm::Module *mod);

std::string val2str(llvm::Value *val);
std::string dbg2str(llvm::Metadata *MD, llvm::Module *mod, llvm::Function *func = NULL);

#endif
