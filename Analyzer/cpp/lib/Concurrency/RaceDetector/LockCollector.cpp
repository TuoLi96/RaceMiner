#include "RaceDetector/LockCollector.h"
#include "Utils/IROperation.h"

/*
 * Implementation of LockCollector
 */

LockCollector::LockCollector(ModPack *mod_pack, IntraAcycleCFG *cfg,
					LockAPI *lock_api, AliasGraph *ag) {
	this->mod_pack = mod_pack;
	this->cfg = cfg;
	this->lock_api = lock_api;
	this->ag = ag;
}

LockCollector::~LockCollector() {
	// Need to do nothing at present.
}

void LockCollector::handleInst(CallInst *lock_inst, CallInst *unlock_inst, Instruction *inst) {
	Value *lock_val = lock_api->getLockVal(lock_inst);
	Value *unlock_val = lock_api->getUnlockVal(unlock_inst);
	if (BinaryOperator *binary_inst = dyn_cast<BinaryOperator>(inst)) {
		for (int op_idx = 0; op_idx < binary_inst->getNumOperands(); op_idx++) {
			Value *op = binary_inst->getOperand(op_idx);
		}
	} else if (CallInst *call_inst = dyn_cast<CallInst>(inst)) {
		for (int arg_idx = 0; arg_idx < call_inst->arg_size(); arg_idx++) {
			Value *arg = call_inst->getArgOperand(arg_idx);
		}	
	} else if (CmpInst *cmp_inst = dyn_cast<CmpInst>(inst)) {
		for (int op_idx = 0; op_idx < cmp_inst->getNumOperands(); op_idx++) {
			Value *op = cmp_inst->getOperand(op_idx);
		}
	} else if (ReturnInst *ret_inst = dyn_cast<ReturnInst>(inst)) {
		Value *ret_val = ret_inst->getReturnValue();
		if (ret_val != NULL) {

		}
	} else if (StoreInst *store_inst = dyn_cast<StoreInst>(inst)) {
		for (int op_idx = 0; op_idx < store_inst->getNumOperands(); op_idx++) {
			Value *op = store_inst->getOperand(op_idx);
		}
	} else if (BranchInst *br_inst = dyn_cast<BranchInst>(inst)) {
		if (br_inst->isConditional()) {
			Value *cond_val = br_inst->getCondition();
		}
	} else if (SwitchInst *sw_inst = dyn_cst<SwithInst>(inst)) {
		Value *cond_val = sw_inst->getCondition();		
	} else {
		// TODO: Handle other instructions.
	}
} 

void LockCollector::handleFunc(Function &func) {
	vector<CallInst *> lock_set;
	for (int topo_idx = 0; topo_idx < cfg->getNumTopoNodes(); topo_idx++) {
		CFGNode *node = cfg->getTopoNode(topo_idx);
		Instruction *inst = node->getInst();
		if (CallInst *call_inst = dyn_cast<CallInst>(inst)) {
			if (lock_api->isLock(call_inst)) {
				lock_set.push_back(call_inst);
			} else if (lock_api->isUnlock(call_inst)) {
				auto lock_it = find_if(
						lock_set.rbegin(), lock_set.rend(), 
						[&](CallInst *lock_call) {
					if (lock_api->isLockPair(lock_call, call_inst)) {
						return true;
					} {
						return false;
					}
				});
				if (delete_it != lock_set.rend()) {
					lock_it = prev(delete_it.base());
					CallInst *lock_call = *lock_it;
					CallInst *unlock_call = call_inst;
					set<CFGNode *> intra_set = cfg->getIntraBetween(lock_call, unlock_call);
					for (auto node : intra_set) {
						Instruction *inst = node->getInst();
						if (inst == lock || inst == unlock_call) {
							continue;
						}
						handleInst(lock_call, unlock_call, inst);
					}
				}
			}
		}
	}
}

void LockCollector::handleMod(Module &mod) {
	for (auto &func : mod) {
		handleFunc(func);
	}
}

void LockCollector::analyze() {
	for (int mgr_idx = 0; mgr_idx < mod_pack->getNumMgrs(); mgr_idx++) {
		analyzing_mod_mgr = mod_pack->getMgr(mgr_idx);
		Module *mod = analyzing_mod_mgr->getMod();
		handleMod(*mod);
	}
}
