#include "AliasAnalysis/SteensgaardInter.h"
#include "Constants.h"
#include "Utils/IROperations.h"

#include <stack>
#include <iostream>

#include "llvm/IR/Constant.h"

using namespace std;
using namespace llvm;

/*
 * Implementation of SteensgaardInter
 */

SteensgaardInter::SteensgaardInter(ModPack *mod_pack) : Steensgaard(mod_pack) {
	this->mod_pack = mod_pack;
	this->analyzing_mod_mgr = NULL;
	Steensgaard::handlePack();
}

SteensgaardInter::~SteensgaardInter() {
	// Need to do nothing at present.
}

void SteensgaardInter::handleCall(CallInst *call_inst) {
}

void SteensgaardInter::handleBlock(BasicBlock &blk) {
	for (auto &inst : blk) {
		if (CallInst *call_inst = dyn_cast<CallInst>(&inst)) {
			handleCall(call_inst);
		}
	}
}

void SteensgaardInter::handleFunc(Function &func) {
	for (auto &blk : func) {
		handleBlock(blk);
	}
}

void SteensgaardInter::handleMod(Module &mod) {
	for (auto &func : mod) {
		handleFunc(func);
	}
}

void SteensgaardInter::handlePack() {
	for (int mod_mgr_idx = 0; mod_mgr_idx < mod_pack->getNumMgrs(); mod_mgr_idx++) {
		analyzing_mod_mgr = mod_pack->getMgr(mod_mgr_idx);
		Module *mod = analyzing_mod_mgr->getMod();
		handleMod(*mod);
	}
}

void SteensgaardInter::build() {
	handlePack();
	compact(uf);
}
