#include "AliasAnalysis/AliasGraph.h"
#include "Constants.h"
#include "Utils/IROperations.h"

#include <queue>
#include <unordered_map>
#include <iostream>
#include <fstream>

#include "llvm/IR/Instructions.h"

using namespace std;
using namespace llvm;

/*
 * Implementation of AGNode
 */

AGNode::AGNode() {
	this->anchor_val = NULL;
	this->anchor_name = "";
	this->type_name = ""; 
	this->alias_set.clear();
	this->in_edges.clear();
	this->out_edges.clear();
}

AGNode::~AGNode() {
	// Need to do nothing at present.
}

void AGNode::setAnchor(llvm::Value *anchor_val, std::string anchor_name) {
	this->anchor_val = anchor_val;
	this->anchor_name = anchor_name;
}

bool AGNode::isAnchor() {
	return anchor_val != NULL;
}

llvm::Value *AGNode::getAnchorVal() {
	return anchor_val;
}

std::string AGNode::getAnchorName() {
	return anchor_name;
}

void AGNode::setTypeName(string type_name) {
	this->type_name = type_name;
}

string AGNode::getTypeName() {
	return type_name;
}

void AGNode::insertVal(Value *val) {
	alias_set.insert(val);
}

const set<Value *> &AGNode::getAliasSet() const {
	return alias_set;
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

void AGNode::pushOutEdge(string field, AGEdge *edge) {
	out_edges[field] = edge;
}

const std::map<string, AGEdge*>& AGNode::getOutEdges() const {
	return out_edges;
}

AGEdge *AGNode::getOutEdge(string field) {
	auto edge_finder = out_edges.find(field);
	if (edge_finder != out_edges.end()) {
		return edge_finder->second;
	} else {
		return NULL;
	}
}

AGNode *AGNode::getOutNode(string field) {
	AGEdge *out_edge = getOutEdge(field);
	if (out_edge == NULL) {
		return NULL;
	} else {
		return out_edge->getDst();
	}
}

string AGNode::toDotNode(size_t id) {
	string node_str;
	node_str += "node" + to_string(id);
	node_str += " [shape=plaintext label=<";
	node_str += "<TABLE BORDER=\"1\" CELLBORDER=\"0\" CELLSPACING=\"0\">";

	node_str += "<TR><TD BGCOLOR=\"lightgray\">";
	node_str += to_string(id);
	node_str += "</TD></TR>";

	node_str += "<TR><TD></TD></TR>";

	for (auto &alias : alias_set) {
		node_str += "<TR><TD ALIGN=\"LEFT\">";
		node_str += alias->getName().str();
		node_str += "</TD></TR>";
	}

	node_str += "</TABLE>";
	node_str += ">];";

	return node_str;
}


/*
 * Implementation of AGEdge
 */

AGEdge::AGEdge(AGNode *src, AGNode *dst, string field) {
	this->src = src;
	this->dst = dst;
	this->field = field;
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

string AGEdge::getField() {
	return field;
}

string AGEdge::toDotEdge(size_t src_id, size_t dst_id) {
	string edge_str;

	edge_str += "node" + to_string(src_id);
	edge_str += " -> ";
	edge_str += "node" + to_string(dst_id);

	edge_str += " [label=\"";
	edge_str += field;
	edge_str += "\"]";

	edge_str += ";";

	return edge_str;
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

unordered_map<AGNode *, int> AliasGraph::getBackDist(AGNode *dst) {
	unordered_map<AGNode *, int> back_dist;
	queue<AGNode *> work_queue;
	back_dist[dst] = 0;
	work_queue.push(dst);

	while (!work_queue.empty()) {
		AGNode *node = work_queue.front();
		work_queue.pop();
		for (int in_idx = 0; in_idx < node->getNumIns(); in_idx++) {
			AGNode *pre_node = node->getInNode(in_idx);
			if (!back_dist.count(pre_node)) {
				back_dist[pre_node] = back_dist[node] + 1;
				work_queue.push(pre_node);
			}
		}
	}
	return back_dist;
}

void AliasGraph::updateAGNode(Value *val, AGNode *agnode) {
	agnode->insertVal(val);
	val2node[val] = agnode;
}

void AliasGraph::createAGEdge(AGNode *src_agnode, AGNode *dst_agnode, string field) {
	AGEdge *new_edge = new AGEdge(src_agnode, dst_agnode, field);
	src_agnode->pushOutEdge(field, new_edge);
	dst_agnode->pushInEdge(new_edge);
	agedge_set.insert(new_edge);
}

AGNode *AliasGraph::getAGNode(Value *val) {
	auto node_find = val2node.find(val);
	if (node_find != val2node.end()) {
		return node_find->second;
	}
	AGNode *new_agnode = new AGNode();
	new_agnode->insertVal(val);
	agnode_set.insert(new_agnode);
	val2node[val] = new_agnode;

	if (isa<AllocaInst>(val) || isa<GlobalVariable>(val)) {
		string type_name = getTypeName(val);
		string var_name = getVarName(val);
		new_agnode->setAnchor(val, var_name);
		new_agnode->setTypeName(type_name);
	}

	return new_agnode;
}

AGNode *AliasGraph::findAGNode(Value *val) {
	auto node_find = val2node.find(val);
	if (node_find != val2node.end()) {
		return node_find->second;
	} else {
		return NULL;
	}
}

void AliasGraph::compact(UnionFind<AGNode *> &uf) {
	// Add all nodes to the union-find set.
	for (AGNode *node : agnode_set) {
		uf.add(node);
	}
	// Initialize the worklist.
	queue<AGNode *> worklist;
	unordered_set<AGNode *> in_worklist;

	for (AGNode *rep : uf.getRoots()) {
		rep = uf.find(rep);
		if (!in_worklist.count(rep)) {
			worklist.push(rep);
			in_worklist.insert(rep);
		}
	}

	// Worklist analysis.
	while (!worklist.empty()) {
		AGNode *rep = worklist.front();
		worklist.pop();

		rep = uf.find(rep);
		in_worklist.erase(rep);

		const auto &members = uf.getMembers(rep);

		unordered_map<string, vector<AGNode *> > field2dsts;

		for (AGNode *node : members) {
			const auto &out_edges = node->getOutEdges();

			for (const auto &[field, edge] : out_edges) {
				if (!edge) {
					continue;
				}

				AGNode *dst = edge->getDst();
				if (!dst) {
					continue;
				}

				field2dsts[field].push_back(dst);
			}
		}

		for (auto &[field, dsts] : field2dsts) {
			if (dsts.size() <= 1) {
				continue;
			}

			AGNode *base_rep = uf.find(dsts[0]);

			for (size_t i = 1; i < dsts.size(); ++i) {
				AGNode *cur_rep = uf.find(dsts[i]);
				base_rep = uf.find(base_rep);

				if (base_rep == cur_rep) {
					continue;
				}

				AGNode *new_rep = uf.unite(base_rep, cur_rep);
				new_rep = uf.find(new_rep);

				if (!in_worklist.count(new_rep)) {
					worklist.push(new_rep);
					in_worklist.insert(new_rep);
				}

				base_rep = new_rep;
			}
		}
	}

	// Create new AGNode.
	unordered_map<AGNode *, AGNode *> rep2newnode;

	for (AGNode *old_node : agnode_set) {
		AGNode *rep = uf.find(old_node);

		if (rep2newnode.find(rep) == rep2newnode.end()) {
			rep2newnode[rep] = new AGNode();
		}

		AGNode *new_node = rep2newnode[rep];
		for (llvm::Value *val : old_node->getAliasSet()) {
			new_node->insertVal(val);
		}

		if (!new_node->isAnchor() && old_node->isAnchor()) {
			new_node->setAnchor(old_node->getAnchorVal(), old_node->getAnchorName());
		}
		if (new_node->getTypeName() == "" && old_node->getTypeName() != "") {
			new_node->setTypeName(old_node->getTypeName());
		}
	}

	// Create new AGNode and AGEdge.
	set<AGNode *> new_agnode_set;
	set<AGEdge *> new_agedge_set;
	llvm::DenseMap<llvm::Value *, AGNode *> new_val2node;

	for (auto &[rep, new_node] : rep2newnode) {
		new_agnode_set.insert(new_node);

		for (llvm::Value *val : new_node->getAliasSet()) {
			new_val2node[val] = new_node;
		}
	}

	struct EdgeKey {
		AGNode *src;
		AGNode *dst;
		string field;

		bool operator==(const EdgeKey &other) const {
			return src == other.src &&
				   dst == other.dst &&
				   field == other.field;
		}
	};

	struct EdgeKeyHash {
		size_t operator()(const EdgeKey &k) const {
			size_t h1 = std::hash<AGNode *>()(k.src);
			size_t h2 = std::hash<AGNode *>()(k.dst);
			size_t h3 = std::hash<string>()(k.field);
			return h1 ^ (h2 << 1) ^ (h3 << 2);
		}
	};

	unordered_set<EdgeKey, EdgeKeyHash> edge_seen;

	for (AGEdge *old_edge : agedge_set) {
		if (!old_edge) {
			continue;
		}

		AGNode *old_src = old_edge->getSrc();
		AGNode *old_dst = old_edge->getDst();
		string field = old_edge->getField();

		if (!old_src || !old_dst) {
			continue;
		}

		AGNode *new_src = rep2newnode[uf.find(old_src)];
		AGNode *new_dst = rep2newnode[uf.find(old_dst)];

		EdgeKey key{new_src, new_dst, field};
		if (edge_seen.count(key)) {
			continue;
		}
		edge_seen.insert(key);

		AGEdge *new_edge = new AGEdge(new_src, new_dst, field);
		new_agedge_set.insert(new_edge);

		new_src->pushOutEdge(field, new_edge);
		new_dst->pushInEdge(new_edge);
	}

	// Release old agnode_set and agedge_set.
	for (AGNode *node : agnode_set) {
		delete node;
	}

	for (AGEdge *edge : agedge_set) {
		delete edge;
	}

	agedge_set = std::move(new_agedge_set);
	agnode_set = std::move(new_agnode_set);
	val2node = std::move(new_val2node);
}

bool AliasGraph::isAlias(Value *val1, Value *val2) {
	AGNode *agnode1 = getAGNode(val1);
	AGNode *agnode2 = getAGNode(val2);
	return agnode1 == agnode2;
}

AGNode *AliasGraph::findNearestAncestor(vector<AGNode *> &nodes, bool should_anchor) {
	if (nodes.empty()) {
		return NULL;
	}
	vector<unordered_map<AGNode *, int> > all_dist;
	for (auto node : nodes) {
		all_dist.push_back(getBackDist(node));
	}

	AGNode *nearest_ancestor = NULL;
	int nearest_dist = INT_MAX;
	for (auto & [node, d0] : all_dist[0]) {
		if (should_anchor && !node->isAnchor()) {
			continue;
		}
		int total_dist = d0;
		bool is_valid = true;
		for (size_t dist_idx = 1; dist_idx < all_dist.size(); dist_idx++) {
			if (!all_dist[dist_idx].count(node)) {
				is_valid = false;
				break;
			}
			total_dist += all_dist[dist_idx][node];
		}
		if (is_valid && total_dist < nearest_dist) {
			nearest_dist = total_dist;
			nearest_ancestor = node;
		}
	}
	return nearest_ancestor;
}

AGNode *AliasGraph::findNearestAncestor(vector<Value *> &vals, bool should_anchor) {
	vector<AGNode *> nodes;
	for (auto val : vals) {
		AGNode *node = getAGNode(val);
		nodes.push_back(node);
	}
	return findNearestAncestor(nodes, should_anchor);
}

vector<AGEdge *> AliasGraph::getFieldPathEdges(AGNode *src, AGNode *dst) {
	if (src == dst) {
		return {};
	}

	unordered_map<AGNode *, AGEdge *> parent_edge;
	queue<AGNode *> work_queue;

	work_queue.push(src);
	parent_edge[src] = nullptr;

	while (!work_queue.empty()) {
		AGNode *node = work_queue.front();
		work_queue.pop();

		if (node == dst) {
			break;
		}

		for (const auto &[field, edge] : node->getOutEdges()) {
			AGNode *out_node = edge->getDst();

			if (!parent_edge.count(out_node)) {
				parent_edge[out_node] = edge;
				work_queue.push(out_node);
			}
		}
	}

	if (!parent_edge.count(dst)) {
		return {};
	}

	vector<AGEdge *> edges;
	for (AGNode *cur = dst; cur != src; ) {
		AGEdge *edge = parent_edge[cur];
		if (!edge) {
			break;
		}

		edges.push_back(edge);
		cur = edge->getSrc();
	}

	reverse(edges.begin(), edges.end());
	return edges;
}

string AliasGraph::getFieldPath(AGNode *src, AGNode *dst) {
	vector<AGEdge *> edges = getFieldPathEdges(src, dst);
	string field_path;

	for (size_t edge_idx = 0; edge_idx < edges.size(); edge_idx++) {
		AGEdge *edge = edges[edge_idx];
		string field = edge->getField();
		if (field == REF_FIELD) {
			continue;
		}
		field_path += ".";
		field_path += field;
	}
	return field_path;
}

void AliasGraph::dumpDot(string dot_file) {
	ofstream ofs(dot_file);

	ofs << "digraph TypeGraph {\n";

	ofs << "rankdir=LR;\n";
	ofs << "splines=true;\n";
	ofs << "overlap=false;\n";
	ofs << "nodesep=0.6;\n";
	ofs << "ranksep=1.0;\n";

	unordered_map<AGNode*, size_t> agnode_id;

	size_t id = 0;

	for (auto agnode : agnode_set) {
		agnode_id[agnode] = id;
		ofs << agnode->toDotNode(id) << "\n";
		id++;
	}

	for (auto agedge : agedge_set) {
		AGNode *src = agedge->getSrc();
		AGNode *dst = agedge->getDst();

		ofs << agedge->toDotEdge(
			agnode_id[src],
			agnode_id[dst]
		) << "\n";
	}

	ofs << "}\n";

	ofs.close();
}

void AliasGraph::dumpSvg(std::string svg_name) {
	string dot = "typegraph.dot";

	dumpDot(dot);

	std::string cmd = "dot -Tsvg " + dot + " -o " + svg_name;

	system(cmd.c_str());
}

