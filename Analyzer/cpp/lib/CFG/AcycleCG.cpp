#include "CFG/AcycleCG.h"

#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <queue>

#include <spdlog/spdlog.h>

using namespace std;
using namespace llvm;

/*
 * Implementation of AcycleCG
 */

AcycleCG::AcycleCG(ModPack *mod_pack) : CG(mod_pack) {
	this->mod_pack = mod_pack;
	this->analyzing_mod_mgr = NULL;
	topo_vec.clear();
}

AcycleCG::~AcycleCG() {
	// Need to do nothing at present.
}

void AcycleCG::breakCycle() {
	struct Frame {
		CGNode *node;
		int next_out_idx;
	};

	auto &node_set = getNodeSet();
	unordered_map<CGNode *, int> color;
	vector<CGEdge *> to_delete;
	unordered_set<CGEdge *> to_delete_set;

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
			CGNode *u = top.node;

			if (top.next_out_idx >= u->getNumOuts()) {
				color[u] = 2;
				stk.pop();
				continue;
			}

			CGEdge *edge = u->getOutEdge(top.next_out_idx);
			top.next_out_idx++;

			if (!edge) {
				continue;
			}

			CGNode *v = edge->getDst();
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

void AcycleCG::topoSort() {
	topo_vec.clear();
	auto &node_set = getNodeSet();
	unordered_map<CGNode *, int> in_degrees;
	queue<CGNode *> work_queue;
	for (auto node : node_set) {
		in_degrees[node] = node->getNumIns();
		if (in_degrees[node] == 0) {
			work_queue.push(node);
		}
	}

	while (!work_queue.empty()) {
		CGNode *node = work_queue.front();
		work_queue.pop();
		topo_vec.push_back(node);

		for (int out_idx = 0; out_idx < node->getNumOuts(); out_idx++) {
			CGNode *out_node = node->getOutNode(out_idx);
			in_degrees[out_node]--;
			if (in_degrees[out_node] == 0) {
				work_queue.push(out_node);
			}
		}
	}

	if (topo_vec.size() != node_set.size()) {
		spdlog::error("AcycleCG::topoSort: Toposort Error!");
	}
} 

void AcycleCG::build() {
	CG::build();
	breakCycle();
	topoSort();
}

int AcycleCG::getNumTopoNodes() {
	return (int)(topo_vec.size());
}

CGNode *AcycleCG::getTopoNode(int topo_idx) {
	return topo_vec[topo_idx];
}
