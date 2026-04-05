#include "Utils/IROperations.h"
#include "Constants.h"

#include "llvm/IR/Value.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/ModuleSlotTracker.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/DebugInfoMetadata.h"

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
			val = cast_inst->getOperand(0);
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

DIVariable *getDbgVar(Value *val) {
	// Process global variables.
	if (GlobalVariable *global_val = dyn_cast<GlobalVariable>(val)) {
		SmallVector<DIGlobalVariableExpression *, 4> dbg_infos;
		global_val->getDebugInfo(dbg_infos);
		DIVariable *first_dbg_var = NULL;
		for (auto *dbg_info : dbg_infos) {
			if (auto *dbg_var = dbg_info->getVariable()) {
				if (!dbg_var->getName().empty()) {
					return dbg_var;
				}
				if (!first_dbg_var) {
					first_dbg_var = dbg_var;
				}
			}
		}
		return first_dbg_var;
	}

	// Process other variables.
	SmallVector<DbgVariableIntrinsic *, 4> dbg_users;
	findDbgUsers(dbg_users, val);
	DIVariable *first_dbg_var = NULL;

	for (auto *DVI : dbg_users) {
		if (isa<DbgDeclareInst>(DVI)) {
			if (auto *dbg_var = DVI->getVariable()) {
				if (!dbg_var->getName().empty()) {
					return dbg_var; 
				}
				if (!first_dbg_var) {
					first_dbg_var = dbg_var;
				}
			}
		}
	}
	for (auto *DVI : dbg_users) {
		if (isa<DbgDeclareInst>(DVI)) {
			if (auto *dbg_var = DVI->getVariable()) {
				if (!dbg_var->getName().empty()) {
					return dbg_var;
				}
				if (!first_dbg_var) {
					first_dbg_var = dbg_var;
				}
			}
		}
	}
	return first_dbg_var;
}

string getSourcePath(Module *mod) {
	if (auto *CUs = mod->getNamedMetadata("llvm.dbg.cu")) {
		for (auto *Op : CUs->operands()) {
			auto *CU = dyn_cast<DICompileUnit>(Op);
			if (!CU) {
				continue;
			}
			DIFile *file = CU->getFile();
			if (!file) {
				continue;
			}
			string mod_path = (file->getDirectory() + "/" + file->getFilename()).str();
			return mod_path;
		}
	}
	return "";
}

static string getRawStr(const function<void(raw_ostream &)> &Fn) {
	string str;
	raw_string_ostream OS(str);
	Fn(OS);
	return OS.str();
}

string val2str(Value *val) {
	if (Instruction *inst = dyn_cast<Instruction>(val)) {
		return getRawStr([&](raw_ostream &OS) {
			inst->print(OS);
		});
	} else if (BasicBlock *blk = dyn_cast<BasicBlock>(val)) {
		return getRawStr([&](raw_ostream &OS) {
			blk->print(OS);
		});
	} else if (Function *func = dyn_cast<Function>(val)) {
		return getRawStr([&](raw_ostream &OS) {
			func->print(OS);
		});
	} else {
		return getRawStr([&](raw_ostream &OS) {
			val->print(OS);
		});
	}
}

string dbg2str(Metadata *MD, Module *mod, Function *func) {
	return getRawStr([&](raw_ostream &OS) {
		ModuleSlotTracker MST(mod);
		if (func) {
			MST.incorporateFunction(*func);
		}

		MD->print(OS, MST, mod);
	});
}

bool isDbgCall(Instruction *inst) {
	CallInst *call_inst = dyn_cast<CallInst>(inst);
	if (call_inst == NULL) {
		return false;
	}
	Function *func = call_inst->getCalledFunction();
	if (func == NULL) {
		return false;
	}
	string func_name = func->getName().str();
	if (func_name.find("llvm.dbg") == string::npos) {
		return false;
	} else {
		return true;
	}
}

bool isConstant(Value *val) {
	return isa<Constant>(val);
}
