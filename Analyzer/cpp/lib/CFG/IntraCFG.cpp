#include "CFG/IntraCFG.h"

#include "llvm/IR/IntrinsicInst.h"

using namespace std;
using namespace llvm;

/*
 * Imeplementation of IntraCFGBuilder
 */

IntraCFG::IntraCFG(ModPack *mod_pack) {
	this->mod_pack = mod_pack;
	this->analyzing_mod_mgr = NULL;
}

IntraCFG::~IntraCFG() {
	// Need to do nothing at present.
}

void IntraCFG::createNodeForBlock(BasicBlock &blk) {
	Instruction *blk_entry_inst = &(*(blk.begin()));
	createCFGNode(blk_entry_inst, CFGNode::NodeType::BlockEntry);
	Instruction *blk_exit_inst = blk.getTerminator();
	if (isa<ReturnInst>(blk_exit_inst)) {
		createCFGNode(blk_exit_inst, CFGNode::NodeType::FuncExit);
	} else {
		createCFGNode(blk_exit_inst, CFGNode::NodeType::BlockExit);
	}

	for (auto &inst : blk) {
		if (!isa<DbgInfoIntrinsic>(&inst)) {
			createCFGNode(&inst, CFGNode::NodeType::Default);
		}
	}
}

void IntraCFG::createNodeForFunc(Function &func) {
	BasicBlock &entry_blk = func.getEntryBlock();
	Instruction *entry_inst = &(*(entry_blk.begin()));
	createCFGNode(entry_inst, CFGNode::NodeType::FuncEntry);
	for (auto &blk : func) {
		createNodeForBlock(blk);
	}
}

void IntraCFG::createNodeForMod(Module &mod) {
	for (auto &func : mod) {
		if (!func.isDeclaration()) {
			createNodeForFunc(func);
		}
	}
}

void IntraCFG::createNodeForPack() {
	for (int mod_mgr_idx = 0; mod_mgr_idx < mod_pack->getNumMgrs(); mod_mgr_idx++) {
		analyzing_mod_mgr = mod_pack->getMgr(mod_mgr_idx);
		Module *mod = analyzing_mod_mgr->getMod();
		createNodeForMod(*mod);
	}
}


void IntraCFG::createEdgeForBlock(BasicBlock &blk) {
	auto blk_iter = blk.begin();
	auto blk_succ_iter = blk.begin();
	blk_succ_iter++;
	for (; blk_succ_iter != blk.end(); blk_succ_iter++) {
		// FIXME: If the first instruction is dbg?
		Instruction *cur_inst = &(*blk_iter);
		Instruction *succ_inst = &(*blk_succ_iter);
		if (isa<DbgInfoIntrinsic>(succ_inst)) {
			continue;
		}
		createCFGEdge(cur_inst, succ_inst, CFGEdge::EdgeType::Block);
		blk_iter = blk_succ_iter;
	}
	Instruction *term_inst = blk.getTerminator();
	if (BranchInst *br_inst = dyn_cast<BranchInst>(term_inst)) {
		for (size_t succ_idx = 0; succ_idx < br_inst->getNumSuccessors(); succ_idx++) {
			BasicBlock *succ_blk = br_inst->getSuccessor(succ_idx);
			Instruction *succ_inst = &(*(succ_blk->begin()));
			createCFGEdge(br_inst, succ_inst, CFGEdge::EdgeType::Branch);
		}
	} else if (SwitchInst *sw_inst = dyn_cast<SwitchInst>(term_inst)) {
		for (size_t succ_idx = 0; succ_idx < sw_inst->getNumSuccessors(); succ_idx++) {
			BasicBlock *succ_blk = sw_inst->getSuccessor(succ_idx);
			Instruction *succ_inst = &(*(succ_blk->begin()));
			createCFGEdge(sw_inst, succ_inst, CFGEdge::EdgeType::Switch);
		}
	}	
}

void IntraCFG::createEdgeForFunc(Function &func) {
	for (auto &blk : func) {
		createEdgeForBlock(blk);
	}
}

void IntraCFG::createEdgeForMod(Module &mod) {
	for (auto &func : mod) {
		createEdgeForFunc(func);
	}
}

void IntraCFG::createEdgeForPack() {
	for (int mod_mgr_idx = 0; mod_mgr_idx < mod_pack->getNumMgrs(); mod_mgr_idx++) {
		analyzing_mod_mgr = mod_pack->getMgr(mod_mgr_idx);
		Module *mod = analyzing_mod_mgr->getMod();
		createEdgeForMod(*mod);
	}
}

void IntraCFG::build() {
	createNodeForPack();
	createEdgeForPack();
}
