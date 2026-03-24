#include "Utils/IROperations.h"
#include "Constants.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"

using namespace std;
using namespace llvm;

vector<ValPathStep> getValPath(Value *val) {
	vector<ValPathStep> path;
	while (true) {
		if (LoadInst *load_inst = dyn_cast<LoadInst>(val)) {
			val = load_inst->getOperand(0);
			path.push_back({load_inst, REF_OFFSET});
		} else if (GetElementPtrInst *gep_inst = dyn_cast<GetElementPtrInst>(val)) {
			val = gep_inst->getOperand(0);
			int offset = GEP_OFFSET;
			Value *offset_val = gep_inst->getOperand(gep_inst->getNumOperands() - 1);
			if (ConstantInt *const_int = dyn_cast<ConstantInt>(offset_val)) {
				if (const_int->getBitWidth() <= 64) {
					offset = const_int->getSExtValue();
				}
			}
			path.push_back({gep_inst, offset});
		} else if (CastInst *cast_inst = dyn_cast<CastInst>(val)) {
			val = gep_inst->getOperand(0);
		} else {
			break;
		}
	}
	if (!isa<AllocaInst>(val)) {
		return {};
	}
	path.push_back({val, VAL_PATH_ROOT});
	reverse(path.begin(), path.end());
	return path;
}

pair<Value *, vector<int> > getOffsetValPath(Value *val) {
	vector<ValPathStep> path = getValPath(val);
	if (path.empty()) {
		return {NULL, {}};
	}
	Value *alloca_val = path[0].val;
	vector<int> offset_path;
	for (size_t path_idx = 1; path_idx < path.size(); path_idx++) {
		offset_path.push_back(path[path_idx].offset);
	}
	return {alloca_val, offset_path};
}
