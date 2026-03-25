#include "Concurrency/RaceDetector/LockCollector.h"
#include "Utils/IROperations.h"

using namespace std;
using namespace llvm;

/*
 * Implementation of LockCollector
 */

LockCollector::LockCollector(ModPack *mod_pack, IntraAcycleCFG *cfg,
					LockAPI *lock_api, AliasGraph *ag, TypeGraph *tg) {
	this->mod_pack = mod_pack;
	this->analyzing_mod_mgr = NULL;
	this->cfg = cfg;
	this->lock_api = lock_api;
	this->ag = ag;
	this->tg = tg;
}

LockCollector::~LockCollector() {
	// Need to do nothing at present.
}

vector<string> LockCollector::getAccessPath(Value *lock_val, 
							Value *unlock_val, Value *access_val) {
	pair<Value *, vector<int> > lock_val_path = getOffsetValPath(lock_val);
	pair<Value *, vector<int> > unlock_val_path = getOffsetValPath(unlock_val);
	pair<Value *, vector<int> > access_val_path = getOffsetValPath(access_val);
	Value *lock_root = lock_val_path.first;
	Value *unlock_root = unlock_val_path.first;
	Value *access_root = access_val_path.first;
	Value *common_root = NULL;
	vector<int> lock_offset_ag_path, unlock_offset_ag_path, access_offset_ag_path;

	if (!lock_root || !unlock_root || !access_root || 
				lock_root != access_root || unlock_root != access_root) {
		vector<Value *> vals = {lock_root, unlock_root, access_root};
		AGNode *ancestor = ag->findNearestAncestor(vals, true);
		if (ancestor == NULL) {
			return {};
		}
		common_root = ancestor->getOneAnchorVal();
		AGNode *lock_node = ag->getAGNode(lock_val);
		AGNode *unlock_node = ag->getAGNode(unlock_val);
		AGNode *access_node = ag->getAGNode(access_val);
		lock_offset_ag_path = ag->getOffsetAGPath(ancestor, lock_node);
		unlock_offset_ag_path = ag->getOffsetAGPath(ancestor, unlock_node);
		access_offset_ag_path = ag->getOffsetAGPath(ancestor, access_node);
	} else {
		common_root = lock_root;
		lock_offset_ag_path = lock_val_path.second;
		unlock_offset_ag_path = unlock_val_path.second;
		access_offset_ag_path = access_val_path.second;
	}
	string lock_path = tg->getTypePath(lock_val, lock_offset_ag_path);
	string unlock_path = tg->getTypePath(unlock_val, unlock_offset_ag_path);
	string access_path = tg->getTypePath(access_val, access_offset_ag_path);
	vector<string> access_paths = {lock_path, unlock_path, access_path};
	return access_paths;
}

void LockCollector::handleInst(CallInst *lock_inst, CallInst *unlock_inst, Instruction *access_inst) {
	Value *lock_val = lock_api->getLockVal(lock_inst);
	Value *unlock_val = lock_api->getUnlockVal(unlock_inst);
	pair<Value *, vector<int> > lock_val_path = getOffsetValPath(lock_val);
	pair<Value *, vector<int> > unlock_val_path = getOffsetValPath(unlock_val);
	if (BinaryOperator *binary_inst = dyn_cast<BinaryOperator>(access_inst)) {
		for (int op_idx = 0; op_idx < binary_inst->getNumOperands(); op_idx++) {
			Value *op = binary_inst->getOperand(op_idx);
			vector<string> access_paths = getAccessPath(lock_val, unlock_val, op);
		}
	} else if (CallInst *call_inst = dyn_cast<CallInst>(access_inst)) {
		for (int arg_idx = 0; arg_idx < call_inst->arg_size(); arg_idx++) {
			Value *arg = call_inst->getArgOperand(arg_idx);
			vector<string> access_paths = getAccessPath(lock_val, unlock_val, arg);
		}	
	} else if (CmpInst *cmp_inst = dyn_cast<CmpInst>(access_inst)) {
		for (int op_idx = 0; op_idx < cmp_inst->getNumOperands(); op_idx++) {
			Value *op = cmp_inst->getOperand(op_idx);
			vector<string> access_paths = getAccessPath(lock_val, unlock_val, op);
		}
	} else if (ReturnInst *ret_inst = dyn_cast<ReturnInst>(access_inst)) {
		Value *ret_val = ret_inst->getReturnValue();
		if (ret_val != NULL) {
			vector<string> access_paths = getAccessPath(lock_val, unlock_val, ret_val);
		}
	} else if (StoreInst *store_inst = dyn_cast<StoreInst>(access_inst)) {
		for (int op_idx = 0; op_idx < store_inst->getNumOperands(); op_idx++) {
			Value *op = store_inst->getOperand(op_idx);
			vector<string> access_paths = getAccessPath(lock_val, unlock_val, op);
		}
	} else if (BranchInst *br_inst = dyn_cast<BranchInst>(access_inst)) {
		if (br_inst->isConditional()) {
			Value *cond_val = br_inst->getCondition();
			vector<string> access_paths = getAccessPath(lock_val, unlock_val, cond_val);
		}
	} else if (SwitchInst *sw_inst = dyn_cast<SwitchInst>(access_inst)) {
		Value *cond_val = sw_inst->getCondition();
		vector<string> access_paths = getAccessPath(lock_val, unlock_val, cond_val);
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
				auto delete_it = find_if(
						lock_set.rbegin(), lock_set.rend(), 
						[&](CallInst *lock_call) {
					if (lock_api->isLockPair(lock_call, call_inst)) {
						return true;
					} {
						return false;
					}
				});
				if (delete_it != lock_set.rend()) {
					auto lock_it = prev(delete_it.base());
					CallInst *lock_call = *lock_it;
					CallInst *unlock_call = call_inst;
					set<CFGNode *> intra_set = cfg->getIntraBetween(lock_call, unlock_call);
					for (auto node : intra_set) {
						Instruction *inst = node->getInst();
						if (inst == lock_call || inst == unlock_call) {
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

void LockCollector::collect() {
	for (int mgr_idx = 0; mgr_idx < mod_pack->getNumMgrs(); mgr_idx++) {
		analyzing_mod_mgr = mod_pack->getMgr(mgr_idx);
		Module *mod = analyzing_mod_mgr->getMod();
		handleMod(*mod);
	}
}
