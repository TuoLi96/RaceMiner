#include "RaceDetector/LockRuleMiner.h"

/*
 * Implementation of LockRuleMiner
 */

LockRuleMiner::LockRuleMiner(ModPack *mod_pack, IntraCFG *cfg) {
	this->mod_pack = mod_pack;
	this->cfg = cfg;
}

LockRuleMiner::~LockRuleMiner() {
	delete cfg;
}

void LockRuleMiner::pushLockFuncInfo(string lock_func, int lock_arg_idx,
							string unlock_func, int unlock_arg_idx) {
	lock_func_vec.push_back(lock_func);
	lock_arg_idx_vec.push_back(lock_arg_idx);
	unlock_func_vec.push_back(unlock_func);
	lock
}

void LockRuleMiner::handleFunc(Function &func) {
	
	for (auto 
}

void LockRuleMiner::handleMod(Module &mod) {
	for (auto &func : mod) {
		handleFunc(func);
	}
}

void LockRuleMiner::analyzer() {
	for (int mgr_idx = 0; mgr_idx < mod_pack->getNumMgrs(); mgr_idx++) {
		analyzing_mod_mgr = mod_pack->getMgr(mgr_idx);
		Module *mod = analyzing_mod_mgr->getMod();
		handleMod(*mod);
	}
}
