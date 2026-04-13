#include "AliasAnalysis/SteensgaardInter.h"
#include "CFG/CG.h"
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

SteensgaardInter::SteensgaardInter(ModPack *mod_pack, CG *cg) : Steensgaard(mod_pack) {
	this->mod_pack = mod_pack;
	this->analyzing_mod_mgr = NULL;
	this->cg = cg;
	Steensgaard::handlePack();
}

SteensgaardInter::~SteensgaardInter() {
	// Need to do nothing at present.
}

void SteensgaardInter::handleCall(CallInst *call_inst) {
	Function *callee = cg->getCallee(call_inst);
	if (callee == NULL || callee->isVarArg()) {
		return;
	}
	if (callee->arg_size() != call_inst->arg_size()) {
		return;
	}
	auto param_it = callee->arg_begin();
	auto arg_it = call_inst->arg_begin();
	for ( ; param_it != callee->arg_end() && arg_it != call_inst->arg_end(); 
					param_it++, arg_it++) {
		AGNode *param_node = getAGNode(&(*param_it));
		AGNode *arg_node = getAGNode(*arg_it);
		uf.unite(param_node, arg_node);
	}
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
