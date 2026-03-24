#ifndef _IR_OPERATION_H_
#define _IR_OPERATION_H_

#include <vector>

#include "llvm/IR/Value.h"

struct ValPathStep {
	llvm::Value *val;
	int offset;
};

std::vector<ValPathStep> getValPath(llvm::Value *val);
std::pair<llvm::Value *, std::vector<int> > getOffsetValPath(llvm::Value *val);

#endif
