#include "CFG/CFG.h"

#include <stack>

#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"

using namespace std;
using namespace llvm;

/*
 * Implementation of CFGNode
 */

CFGNode::CFGNode(Instruction *inst, CFGNode::NodeType node_type) {
	this->inst = inst;
	this->line = getLine(inst);
	this->node_type = node_type;
	this->in_edges.clear();
	this->out_edges.clear();
}

CFGNode::~CFGNode() {
	// Need to do nothing at present;
}

int CFGNode::getLine(Instruction *inst) {
	if (auto &dbg_loc = inst->getDebugLoc()) {
		return dbg_loc.getLine();
	}
	if (auto *alloca_inst = dyn_cast<AllocaInst>(inst)) {
		for (auto *user : alloca_inst->users()) {
			if (auto *dbg_dec_inst = dyn_cast<DbgDeclareInst>(user)) {
				auto *dbg_var = dbg_dec_inst->getVariable();
				if (dbg_var) {
					return dbg_var->getLine();
				}
			}
		}
	}
	for (auto *prev_inst = inst->getPrevNode(); 
				prev_inst; prev_inst = prev_inst->getPrevNode()) {
		if (auto &dbg_loc = prev_inst->getDebugLoc()) {
			return dbg_loc.getLine();
		}
	}
	for (auto *next_inst = inst->getNextNode();
				next_inst; next_inst = next_inst->getNextNode()) {
		if (auto &dbg_loc = next_inst->getDebugLoc()) {
			return dbg_loc.getLine();
		}
	}
	return 0;
}

Instruction *CFGNode::getInst() {
	return inst;
}

int CFGNode::getLine() {
	return line;
}

CFGNode::NodeType CFGNode::getNodeType() {
	return node_type;
}

void CFGNode::pushInEdge(CFGEdge *edge) {
	in_edges.push_back(edge);
}

int CFGNode::getNumIns() {
	return (int)(in_edges.size());
}

CFGEdge *CFGNode::getInEdge(int in_idx) {
	return in_edges[in_idx];
}

CFGNode *CFGNode::getInNode(int in_idx) {
	return in_edges[in_idx]->getSrc();
}

void CFGNode::pushOutEdge(CFGEdge *edge) {
	out_edges.push_back(edge);
}

int CFGNode::getNumOuts() {
	return (int)(out_edges.size());
}

CFGEdge *CFGNode::getOutEdge(int out_idx) {
	return out_edges[out_idx];
}

CFGNode *CFGNode::getOutNode(int out_idx) {
	return out_edges[out_idx]->getDst();
}


/* 
 * Implementation of CFGEdge
 */

CFGEdge::CFGEdge(CFGNode *src, CFGNode *dst, CFGEdge::EdgeType edge_type) {
	this->src = src;
	this->dst = dst;
	this->edge_type = edge_type;
}

CFGNode *CFGEdge::getSrc() {
	return src;
}

CFGNode *CFGEdge::getDst() {
	return dst;
}

CFGEdge::EdgeType CFGEdge::getEdgeType() {
	return edge_type;
}

/*
 * Implementation of CFG
 */

CFG::CFG() {
	node_set.clear();
	edge_set.clear();
	inst2node.clear();
	entry_vec.clear();
}

CFG::~CFG() {
	for (auto &node : node_set) {
		delete node;
	}
	for (auto &edge : edge_set) {
		delete edge;
	}
}

set<CFGNode *> CFG::getSuccs(CFGNode *src_node, bool is_intra) {
	stack<CFGNode *> worklist;
	set<CFGNode *> result;

	worklist.push(src_node);
	while (!worklist.empty()) {
		CFGNode *node = worklist.top();
		worklist.pop();
		if (result.find(node) != result.end()) {
			continue;
		} else {
			result.insert(node);
		}
		for (int out_idx = 0; out_idx < node->getNumOuts(); out_idx++) {
			if (!is_intra) {
				worklist.push(node->getOutNode(out_idx));
				continue;
			}
			CFGEdge *out_edge = node->getOutEdge(out_idx);
			// TODO: This condition should be rechecked carefully for intra-CFG.
			if (out_edge->getEdgeType() == CFGEdge::EdgeType::Ret ||
					out_edge->getEdgeType() == CFGEdge::EdgeType::Call) {
				continue;
			}
			worklist.push(out_edge->getDst());
		}
	}
	return result;
}

set<CFGNode *> CFG::getPreds(CFGNode *dst_node, bool is_intra) {
	stack<CFGNode *> worklist;
	set<CFGNode *> result;

	worklist.push(dst_node);
	while (!worklist.empty()) {
		CFGNode *node = worklist.top();
		worklist.pop();
		if (result.find(node) != result.end()) {
			continue;
		} else {
			result.insert(node);
		}
		for (int in_idx = 0; in_idx < node->getNumIns(); in_idx++) {
			if (!is_intra) {
				worklist.push(node->getInNode(in_idx));
				continue;
			}
			CFGEdge *in_edge = node->getInEdge(in_idx);
			// TODO: This condition should be rechecked carefully for intra-CFG.
			if (in_edge->getEdgeType() == CFGEdge::EdgeType::Ret ||
					in_edge->getEdgeType() == CFGEdge::EdgeType::Call) {
				continue;
			}
			worklist.push(in_edge->getSrc());
		}
	}
	return result;
} 

set<CFGNode *> CFG::getIntraSuccs(CFGNode *src_node) {
	return getSuccs(src_node, true);
}

set<CFGNode *> CFG::getIntraPreds(CFGNode *dst_node) {
	return getPreds(dst_node, true);
}

set<CFGNode *> CFG::getInterSuccs(CFGNode *src_node) {
	return getSuccs(src_node, false);
}

set<CFGNode *> CFG::getInterPreds(CFGNode *dst_node) {
	return getPreds(dst_node, false);
}

void CFG::createCFGNode(Instruction *inst, CFGNode::NodeType node_type) {
	if (inst2node.find(inst) != inst2node.end()) {
		return;
	}
	CFGNode *new_node = new CFGNode(inst, node_type);
	inst2node[inst] = new_node;
	node_set.insert(new_node);
}

void CFG::createCFGEdge(Instruction *src_inst, Instruction *dst_inst,
					CFGEdge::EdgeType edge_type) {
	CFGNode *src_node = getCFGNode(src_inst);
	CFGNode *dst_node = getCFGNode(dst_inst);
	CFGEdge *edge = new CFGEdge(src_node, dst_node, edge_type);
	edge_set.insert(edge);
}

void CFG::collectEntry() {
	entry_vec.clear();
	for (auto &node : node_set) {
		if (node->getNumIns() == 0) {
			entry_vec.push_back(node);
		}
	}
}

CFGNode *CFG::getCFGNode(Instruction *inst) {
	return inst2node[inst];
}

set<CFGNode *> CFG::getIntraBetween(CFGNode *src_node, CFGNode *dst_node) {
	set<CFGNode *> src_succs = getIntraSuccs(src_node);
	set<CFGNode *> dst_preds = getIntraPreds(dst_node);
	set<CFGNode *> result;
	set_intersection(src_succs.begin(), src_succs.end(),
					dst_preds.begin(), dst_preds.end(),
					inserter(result, result.begin()));
	return result;
}

set<CFGNode *> CFG::getInterBetween(CFGNode *src_node, CFGNode *dst_node) {
	set<CFGNode *> src_succs = getInterSuccs(src_node);
	set<CFGNode *> dst_preds = getInterPreds(dst_node);
	set<CFGNode *> result;
	set_intersection(src_succs.begin(), src_succs.end(),
					dst_preds.begin(), dst_preds.end(),
					inserter(result, result.begin()));
	return result;
}

int CFG::getNumEntries() {
	return (int)(entry_vec.size());
}

CFGNode *CFG::getEntry(int entry_idx) {
	return entry_vec[entry_idx];
}
