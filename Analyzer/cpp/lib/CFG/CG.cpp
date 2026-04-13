#include "CFG/CG.h"

using namespace std;
using namespace llvm;

/*
 * Implementation of CGNode
 */

CGNode::CGNode(Function *func) {
	this->func = func;
	this->in_edges.clear();
	this->out_edges.clear();
}

CGNode::~CGNode() {
	// Need to do nothing at present.
}

Function *CGNode::getFunc() {
	return func;
}

void CGNode::pushInEdge(CGEdge *edge) {
	in_edges.push_back(edge);
}

void CGNode::deleteInEdge(CGEdge *edge) {
	for (size_t edge_idx = 0; edge_idx < in_edges.size(); edge_idx++) {
		if (in_edges[edge_idx] == edge) {
			in_edges[edge_idx] = in_edges.back();
			in_edges.pop_back();
			break;
		}
	}
}

int CGNode::getNumIns() {
	return (int)(in_edges.size());
}

CGEdge *CGNode::getInEdge(int in_idx) {
	return in_edges[in_idx];
}

CGNode *CGNode::getInNode(int in_idx) {
	return in_edges[in_idx]->getSrc();
}

void CGNode::pushOutEdge(CGEdge *edge) {
	out_edges.push_back(edge);
}

void CGNode::deleteOutEdge(CGEdge *edge) {
	for (size_t edge_idx = 0; edge_idx < out_edges.size(); edge_idx++) {
		if (out_edges[edge_idx] == edge) {
			out_edges[edge_idx] = out_edges.back();
			out_edges.pop_back();
			break;
		}
	}
}

int CGNode::getNumOuts() {
	return (int)(out_edges.size());
}

CGEdge *CGNode::getOutEdge(int out_idx) {
	return out_edges[out_idx];
}

CGNode *CGNode::getOutNode(int out_idx) {
	return out_edges[out_idx]->getSrc();
}


/*
 * Implementation of CGEdge.
 */

CGEdge::CGEdge(CGNode *src, CGNode *dst, CallInst *call_inst, CGEdge::EdgeType edge_type) {
	this->src = src;
	this->dst = dst;
	this->call_inst = call_inst;
	this->edge_type = edge_type;
}

CGNode *CGEdge::getSrc() {
	return src;
}

CGNode *CGEdge::getDst() {
	return dst;
}

CallInst *CGEdge::getCallInst() {
	return call_inst;
}

CGEdge::EdgeType CGEdge::getEdgeType() {
	return edge_type;
}


/*
 * Implemantation of CG.
 */

CG::CG(ModPack *mod_pack) {
	this->mod_pack = mod_pack;
	this->analyzing_mod_mgr = 0;
	node_set.clear();
	edge_set.clear();
	func2node.clear();
	call2edge.clear();
	entry_vec.clear();
	entry_set.clear();
}

CG::~CG() {
	for (auto &node : node_set) {
		delete node;
	}
	for (auto &edge : edge_set) {
		delete edge;
	}
}

void CG::createCGNode(Function *func) {
	if (func2node.find(func) != func2node.end()) {
		return;
	}
	CGNode *new_node = new CGNode(func);
	func2node[func] = new_node;
	node_set.insert(new_node);
}

void CG::createCGEdge(Function *src_func, Function *dst_func, 
						CallInst *call_inst, CGEdge::EdgeType edge_type) {
	CGNode *src_node = getCGNode(src_func);
	CGNode *dst_node = getCGNode(dst_func);
	CGEdge *edge = new CGEdge(src_node, dst_node, call_inst, edge_type);
	src_node->pushOutEdge(edge);
	dst_node->pushInEdge(edge);
	edge_set.insert(edge);
}

void CG::collectEntry() {
	entry_vec.clear();
	entry_set.clear();
	for (auto &node : node_set) {
		if (node->getNumIns() == 0) {
			entry_vec.push_back(node);
			entry_set.insert(node);
		}
	}
}

void CG::deleteEdge(CGEdge *edge) {
	CGNode *src_node = edge->getSrc();
	CGNode *dst_node = edge->getDst();
	src_node->deleteOutEdge(edge);
	dst_node->deleteInEdge(edge);
	edge_set.erase(edge);
	delete edge;
} 

void CG::deleteEdge(Function *src_func, Function *dst_func) {
	CGNode *src_node = getCGNode(src_func);
	CGNode *dst_node = getCGNode(dst_func);
	for (auto edge : edge_set) {
		if (edge->getSrc() == src_node && edge->getDst() == dst_node) {
			deleteEdge(edge);
			break;
		}
	}
}

Function *CG::getFuncByName(string func_name) {
	auto func_find = name2func.find(func_name);
	if (func_find != name2func.end()) {
		return func_find->second;
	} else {
		return NULL;
	}
}

void CG::createNodeForMod(Module &mod) {
	for (auto &func : mod) {
		if (!func.isDeclaration()) {
			createCGNode(&func);
			if (!func.hasInternalLinkage()) {
				string func_name = func.getName().str();
				name2func[func_name] = &func;
			}
		}
	}
}

void CG::createNodeForPack() {
	for (int mod_mgr_idx = 0; mod_mgr_idx < mod_pack->getNumMgrs(); mod_mgr_idx++) {
		analyzing_mod_mgr = mod_pack->getMgr(mod_mgr_idx);
		Module *mod = analyzing_mod_mgr->getMod();
		createNodeForMod(*mod);
	}
}

void CG::createEdgeForCallInst(CallInst *call_inst) {
	Function *callee = call_inst->getCalledFunction();
	if (callee == NULL) {
		return;
	}
	if (callee->isDeclaration()) {
		callee = getFuncByName(callee->getName().str());
	}
	if (callee == NULL) {
		return;
	}
	createCGEdge(call_inst->getFunction(), callee, call_inst, CGEdge::EdgeType::Direct);
}

void CG::createEdgeForBlock(BasicBlock &blk) {
	for (auto &inst : blk) {
		if (CallInst *call_inst = dyn_cast<CallInst>(&inst)) {
			createEdgeForCallInst(call_inst);
		}
	}
} 

void CG::createEdgeForFunc(Function &func) {
	for (auto &blk : func) {
		createEdgeForBlock(blk);
	}
}

void CG::createEdgeForMod(Module &mod) {
	for (auto &func : mod) {
		createEdgeForFunc(func);
	}
}

void CG::createEdgeForPack() {
	for (int mod_mgr_idx = 0; mod_mgr_idx < mod_pack->getNumMgrs(); mod_mgr_idx++) {
		analyzing_mod_mgr = mod_pack->getMgr(mod_mgr_idx);
		Module *mod = analyzing_mod_mgr->getMod();
		createEdgeForMod(*mod);
	}
}

void CG::build() {
	createNodeForPack();
	createEdgeForPack();
}

CGNode *CG::getCGNode(Function *func) {
	auto node_find = func2node.find(func);
	if (node_find != func2node.end()) {
		return node_find->second;
	} else {
		return NULL;
	}
}

CGEdge *CG::getCGEdge(CallInst *call_inst) {
	auto edge_find = call2edge.find(call_inst);
	if (edge_find != call2edge.end()) {
		return edge_find->second;
	} else {
		return NULL;
	}
}

Function *CG::getCallee(CallInst *call_inst) {
	CGEdge *edge = getCGEdge(call_inst);
	if (edge == NULL) {
		return NULL;
	} else {
		return edge->getDst()->getFunc();
	}
}

int CG::getNumEntries() {
	return (int)(entry_vec.size());
}

CGNode *CG::getEntry(int entry_idx) {
	return entry_vec[entry_idx];
}
