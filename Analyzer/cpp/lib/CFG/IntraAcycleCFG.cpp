#include "CFG/IntraAcycleCFG.h"

#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"

#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <queue>

#include <spdlog/spdlog.h>

using namespace std;
using namespace llvm;

/*
 * Implementation of IntraAcycleCFG
 */

IntraAcycleCFG::IntraAcycleCFG(ModPack *mod_pack) : IntraCFG(mod_pack) {
	this->mod_pack = mod_pack;
	this->analyzing_mod_mgr = NULL;
	topo_vec.clear();
}

IntraAcycleCFG::~IntraAcycleCFG() {
	// Need to do nothing at present.
}

void IntraAcycleCFG::breakRemainCycle() {
	struct Frame {
		CFGNode *node;
		int next_out_idx;
	};

	auto &node_set = getNodeSet();
	unordered_map<CFGNode *, int> color;
	vector<CFGEdge *> to_delete;
	unordered_set<CFGEdge *> to_delete_set;

	for (auto node : node_set) {
		color[node] = 0;
	}

	for (auto start : node_set) {
		if (!start || color[start] != 0) {
			continue;
		}
		stack<Frame> stk;
		stk.push({start, 0});
		color[start] = 1;

		while (!stk.empty()) {
			Frame &top = stk.top();
			CFGNode *u = top.node;

			if (top.next_out_idx >= u->getNumOuts()) {
				color[u] = 2;
				stk.pop();
				continue;
			}

			CFGEdge *edge = u->getOutEdge(top.next_out_idx);
			top.next_out_idx++;

			if (!edge) {
				continue;
			}

			CFGNode *v = edge->getDst();
			if (!v) {
				continue;
			}

			if (color[v] == 0) {
				stk.push({v, 0});
				color[v] = 1;
			} else if (color[v] == 1) {
				if (!to_delete_set.count(edge)) {
					to_delete.push_back(edge);
					to_delete_set.insert(edge);
				}
			}
		}
	}
	for (auto edge : to_delete) {
		deleteEdge(edge);
	}
}

void IntraAcycleCFG::breakCycleForFunc(Function &func) {
	if (func.isDeclaration()) {
		return;
	}
	DominatorTree DT(func);
	for (auto &blk : func) {
		BranchInst *br_inst = dyn_cast<BranchInst>(blk.getTerminator());
		if (!br_inst) {
			continue;
		}
		for (size_t succ_idx = 0; succ_idx < br_inst->getNumSuccessors(); succ_idx++) {
			BasicBlock *succ = br_inst->getSuccessor(succ_idx);
			if (succ != &blk && DT.dominates(succ, &blk)) {
				Instruction *succ_inst = &(*(succ->begin()));
				deleteEdge(br_inst, succ_inst);
			}
		}
	}
}

void IntraAcycleCFG::breakCycleForMod(Module &mod) {
	for (auto &func : mod) {
		breakCycleForFunc(func);
	}
}

void IntraAcycleCFG::breakCycle() {
	for (int mod_mgr_idx = 0; mod_mgr_idx < mod_pack->getNumMgrs(); mod_mgr_idx++) {
		analyzing_mod_mgr = mod_pack->getMgr(mod_mgr_idx);
		Module *mod = analyzing_mod_mgr->getMod();
		breakCycleForMod(*mod);
	}
	collectEntry();
}

void IntraAcycleCFG::topoSort() {
	topo_vec.clear();
	auto &node_set = getNodeSet();
	unordered_map<CFGNode *, int> in_degrees;
	queue<CFGNode *> work_queue;
	for (auto node : node_set) {
		in_degrees[node] = node->getNumIns();
		if (in_degrees[node] == 0) {
			work_queue.push(node);
		}
	}

	while (!work_queue.empty()) {
		CFGNode *node = work_queue.front();
		work_queue.pop();
		topo_vec.push_back(node);
		func2topo_vec[node->getInst()->getFunction()].push_back(node);

		for (int out_idx = 0; out_idx < node->getNumOuts(); out_idx++) {
			CFGNode *out_node = node->getOutNode(out_idx);
			in_degrees[out_node]--;
			if (in_degrees[out_node] == 0) {
				work_queue.push(out_node);
			}
		}
	}

	if (topo_vec.size() != node_set.size()) {
		spdlog::error("IntraAcycleCFG::topoSort: Toposort Error!");
	}
} 

void IntraAcycleCFG::build() {
	IntraCFG::build();
	breakCycle();
	breakRemainCycle();
	topoSort();
}

int IntraAcycleCFG::getNumTopoNodes() {
	return (int)(topo_vec.size());
}

CFGNode *IntraAcycleCFG::getTopoNode(int topo_idx) {
	return topo_vec[topo_idx];
}

vector<CFGNode *> &IntraAcycleCFG::getTopoVecOfFunc(Function *func) {
	return func2topo_vec[func];
}
