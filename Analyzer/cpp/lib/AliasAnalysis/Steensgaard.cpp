#include "AliasAnalysis/Steensgaard.h"
#include "Constants.h"
#include "Utils/IROperations.h"

#include <stack>
#include <iostream>

#include "llvm/IR/Constant.h"

using namespace std;
using namespace llvm;

/*
 * Implementation of Steensgaard
 */

Steensgaard::Steensgaard(ModPack *mod_pack) {
	this->mod_pack = mod_pack;
	this->analyzing_mod_mgr = NULL;
}

Steensgaard::~Steensgaard() {
	// Need to do nothing at present.
}

void Steensgaard::unify(AGNode *agnode1, AGNode *agnode2) {
	stack<AGNode *> node_stk1;
	stack<AGNode *> node_stk2;
	node_stk1.push(agnode1);
	node_stk2.push(agnode2);
	set<AGNode *> node_rec1;
	while (!node_stk1.empty()) {
		agnode1 = node_stk1.top();
		agnode2 = node_stk2.top();
		node_stk1.pop();
		node_stk2.pop();
		if (node_rec1.find(agnode1) != node_rec1.end()) {
			continue;
		} else {
			node_rec1.insert(agnode1);
		}
		uf.unite(agnode1, agnode2);
		for (const auto &kv : agnode1->getOutEdges()) {
			int offset = kv.first;
			AGNode *dst_node1 = kv.second->getDst();
			AGNode *dst_node2 = agnode2->getOutNodeByOffset(offset);
			if (dst_node2) {
				node_stk1.push(dst_node1);
				node_stk2.push(dst_node2);
			}
		}
	}
}


void Steensgaard::createNodeForInst(Instruction *inst) {
	if (isDbgCall(inst)) {
		return;
	}
	for (size_t op_idx = 0; op_idx < inst->getNumOperands(); op_idx++) {
		Value *op = inst->getOperand(op_idx);
		if (isa<BasicBlock>(op) || isa<Constant>(op)) {
			// NOTE: Skip contant at present.
			continue;
		}
		createAGNode(op);
	}
	if (!isa<StoreInst>(inst) && !isa<ReturnInst>(inst) && 
				!isa<BranchInst>(inst) && !isa<SwitchInst>(inst) &&
				!isa<UnreachableInst>(inst)) {
		createAGNode(inst);
	}
}

void Steensgaard::createNodeForBlock(BasicBlock &blk) {
	for (auto &inst : blk) {
		createNodeForInst(&inst);
	}
}

void Steensgaard::createNodeForFunc(Function &func) {
	for (auto &blk : func) {
		createNodeForBlock(blk);
	}
}

void Steensgaard::createNodeForMod(Module &mod) {
	for (auto &func : mod) {
		createNodeForFunc(func);
	}
}

void Steensgaard::createNodeForPack() {
	for (int mod_mgr_idx = 0; mod_mgr_idx < mod_pack->getNumMgrs(); mod_mgr_idx++) {
		analyzing_mod_mgr = mod_pack->getMgr(mod_mgr_idx);
		Module *mod = analyzing_mod_mgr->getMod();
		createNodeForMod(*mod);
	}
}

void Steensgaard::handleLoad(LoadInst *load_inst) {
	Value *pointer = load_inst->getOperand(0);
	AGNode *pointer_node = getAGNode(pointer);
	AGNode *val_node = getAGNode(load_inst);
	AGNode *succ_node = pointer_node->getOutNodeByOffset(REF_OFFSET);
	if (succ_node != NULL) {
		uf.unite(val_node, succ_node);
	} else {
		createAGEdge(pointer_node, val_node, REF_OFFSET);
	}
}

void Steensgaard::handleStore(StoreInst *store_inst) {
	Value *pointer = store_inst->getOperand(1);
	Value *val = store_inst->getOperand(0);
	if (isa<Constant>(val)) {
		return;
	}
	AGNode *pointer_node = getAGNode(pointer);
	AGNode *val_node = getAGNode(val);
	AGNode *succ_node = pointer_node->getOutNodeByOffset(REF_OFFSET);
	if (succ_node != NULL) {
		uf.unite(val_node, succ_node);
	} else {
		createAGEdge(pointer_node, val_node, REF_OFFSET);
	}
}

void Steensgaard::handleGep(GetElementPtrInst *gep_inst) {
	Value *base = gep_inst->getOperand(0);
	Value *val = gep_inst;
	Value *offset_val = gep_inst->getOperand(gep_inst->getNumOperands() - 1);
	int offset = GEP_OFFSET;
	if (ConstantInt *const_int = dyn_cast<ConstantInt>(offset_val)) {
		if (const_int->getBitWidth() <= 64) {
			offset = const_int->getSExtValue();
		}
	}
	AGNode *base_node = getAGNode(base);
	AGNode *val_node = getAGNode(val);
	AGNode *offset_node = base_node->getOutNodeByOffset(offset);
	if (offset_node != NULL) {
		uf.unite(val_node, offset_node);
	} else {
		createAGEdge(base_node, val_node, offset);
	}
}

void Steensgaard::handleCast(BitCastInst *cast_inst) {
	Value *ori_val = cast_inst->getOperand(0);
	if (isa<Constant>(ori_val)) {
		return;
	}
	Value *tar_val = cast_inst;
	AGNode *ori_node = getAGNode(ori_val);
	AGNode *tar_node = getAGNode(tar_val);
	uf.unite(ori_node, tar_node);
}

void Steensgaard::handleInst(Instruction *inst) {
	if (LoadInst *load_inst = dyn_cast<LoadInst>(inst)) {
		handleLoad(load_inst);
	} else if (StoreInst *store_inst = dyn_cast<StoreInst>(inst)) {
		handleStore(store_inst);
	} else if (GetElementPtrInst *gep_inst = dyn_cast<GetElementPtrInst>(inst)) {
		handleGep(gep_inst);
	} else if (BitCastInst *cast_inst = dyn_cast<BitCastInst>(inst)) {
		handleCast(cast_inst);
	} else {
		// TODO: Handle other instructions.
	}
}

void Steensgaard::handleBlock(BasicBlock &blk) {
	for (auto &inst : blk) {
		handleInst(&inst);
	}
}

void Steensgaard::handleFunc(Function &func) {
	for (auto &blk : func) {
		handleBlock(blk);
	}
}

void Steensgaard::handleMod(Module &mod) {
	for (auto &func : mod) {
		handleFunc(func);
	}
}

void Steensgaard::handlePack() {
	for (int mod_mgr_idx = 0; mod_mgr_idx < mod_pack->getNumMgrs(); mod_mgr_idx++) {
		analyzing_mod_mgr = mod_pack->getMgr(mod_mgr_idx);
		Module *mod = analyzing_mod_mgr->getMod();
		handleMod(*mod);
	}
}

void Steensgaard::build() {
	createNodeForPack();
	handlePack();
	compact(uf);
}
