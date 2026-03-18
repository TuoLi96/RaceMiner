#include "RaceDetector/LockRuleMiner.h"

/*
 * Implementation of LockRuleMiner
 */

LockRuleMiner::LockRuleMiner(ModPack *mod_pack) {
	this->mod_pack = mod_pack;
}

LockRuleMiner::~LockRuleMiner() {
	delete cfg;
}

void LockRuleMiner::handleFunc(Function *func) {
	
}

void LockRuleMiner::handleMod(Module *mod) {
	for (auto &func : *mod) {
		handleFunc(func);
	}
}

void LockRuleMiner::analyzer() {
	this->cfg = new IntraCFG(mod_pack);
	for (int mgr_idx = 0; mgr_idx < mod_pack->getNumMgrs(); mgr_idx++) {
		analyzing_mod_mgr = mod_pack->getMgr(mgr_idx);
		Module *mod = analyzing_mod_mgr->getMod();
		handleMod(mod);
	}
}
