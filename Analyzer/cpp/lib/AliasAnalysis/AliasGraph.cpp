#include "AliasAnalysis/AliasGraph.h"

using namespace std;
using namespace llvm;

/*
 * Implementation of AGNode
 */

AGNode::AGNode() {
	this->alias_set.clear();
	this->in_edges.clear();
	this->out_edges.clear();
}

AGNode::~AGNode() {
	// Need to do nothing at present.
}

void AGNode::insertVal(Value *val) {
	alias_set.insert(val);
}

void AGNode::pushInEdge(AGEdge *edge) {
	in_edges.push_back(edge);
}

int AGNode::getNumIns() {
	return (int)(in_edges.size());
}

AGEdge *AGNode::getInEdge(int in_idx) {
	return in_edges[in_idx];
}

AGNode *AGNode::getInNode(int in_idx) {
	return in_edges[in_idx]->getSrc();
}

void AGNode::pushOutEdge(int offset, AGEdge *edge) {
	out_edges[offset] = edge;
}

const std::map<int, AGEdge*>& AGNode::getOutEdges() const {
	return out_edges;
}

AGEdge *AGNode::getOutEdgeByOffset(int offset) {
	auto edge_finder = out_edges.find(offset);
	if (edge_finder != out_edges.end()) {
		return edge_finder->second;
	} else {
		return NULL;
	}
}

AGNode *AGNode::getOutNodeByOffset(int offset) {
	AGEdge *out_edge = getOutEdgeByOffset(offset);
	if (out_edge == NULL) {
		return NULL;
	} else {
		return out_edge->getDst();
	}
}


/*
 * Implementation of AGEdge
 */

AGEdge::AGEdge(AGNode *src, AGNode *dst, int offset) {
	this->src = src;
	this->dst = dst;
	this->offset = offset;
}

AGEdge::~AGEdge() {
	// Need to do nothing at present.
}

AGNode *AGEdge::getSrc() {
	return src;
}

AGNode *AGEdge::getDst() {
	return dst;
}

int AGEdge::getOffset() {
	return offset;
}


/*
 * Implementation of AliasGraph
 */

AliasGraph::AliasGraph() {
	this->agnode_set.clear();
	this->agedge_set.clear();
	this->val2node.clear();
}

AliasGraph::~AliasGraph() {
	for (auto &agnode : agnode_set) {
		delete agnode;
	}
	for (auto &agedge : agedge_set) {
		delete agedge;
	}
}

void AliasGraph::createAGNode(Value *val) {
	if (val2node.find(val) != val2node.end()) {
		return;
	}
	AGNode *new_agnode = new AGNode();
	new_agnode->insertVal(val);
	agnode_set.insert(new_agnode);
	val2node[val] = new_agnode;
}

void AliasGraph::createAGEdge(AGNode *src_agnode, AGNode *dst_agnode, int offset) {
	AGEdge *new_edge = new AGEdge(src_agnode, dst_agnode, offset);
	src_agnode->pushOutEdge(offset, new_edge);
	dst_agnode->pushInEdge(new_edge);
	agedge_set.insert(new_edge);
}

AGNode *AliasGraph::getAGNode(Value *val) {
	return val2node[val];
}

void AliasGraph::compact(UnionFind<AGNode *> &uf) {
	DenseMap<AGNode *, AGNode *> new_agnode_map;
	set<AGNode *> old_agnode_set = std::move(agnode_set);
	set<AGEdge *> old_agedge_set = std::move(agedge_set);
	DenseMap<Value *, AGNode *> old_val2node = std::move(val2node);
	agnode_set.clear();
	agedge_set.clear();
	val2node.clear();
	// Alloca new AGNode for each representative AGNode.
	// Map each old AGNode to the new one.
	for (auto &old_agnode : old_agnode_set) {
		AGNode *rep_old_agnode = uf.find(old_agnode);
		auto new_agnode_finder = new_agnode_map.find(rep_old_agnode);
		if (new_agnode_finder != new_agnode_map.end()) {
			new_agnode_map[old_agnode] = new_agnode_finder->second;
		} else {
			AGNode *new_agnode = new AGNode();
			new_agnode_map[old_agnode] = new_agnode;
			new_agnode_map[rep_old_agnode] = new_agnode;
			agnode_set.insert(new_agnode);
		}
	}
	// Rebuild AGEdges
	for (auto &old_agedge : old_agedge_set) {
		AGNode *old_src = old_agedge->getSrc();
		AGNode *old_dst = old_agedge->getDst();
		int offset = old_agedge->getOffset();
		AGNode *new_src = new_agnode_map[old_src];
		AGNode *new_dst = new_agnode_map[old_dst];
		AGEdge *new_edge = new AGEdge(new_src, new_dst, offset);
		new_src->pushOutEdge(offset, new_edge);
		new_dst->pushInEdge(new_edge);
		agedge_set.insert(new_edge);
	}
	// Map Values to new AGNode
	for (auto &kv : old_val2node) {
		Value *val = kv.first;
		AGNode *old_agnode = kv.second;
		AGNode *new_agnode = new_agnode_map[old_agnode];
		val2node[val] = new_agnode;
		new_agnode->insertVal(val);
	}
	// Release old AGNode and AGEdge
	for (auto &old_agnode : old_agnode_set) {
		delete old_agnode;
	}
	for (auto &old_agedge : old_agedge_set) {
		delete old_agedge;
	}
}

bool AliasGraph::isAlias(Value *val1, Value *val2) {
	AGNode *agnode1 = getAGNode(val1);
	AGNode *agnode2 = getAGNode(val2);
	return agnode1 == agnode2;
}
