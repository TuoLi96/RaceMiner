#ifndef _CFG_H_
#define _CFG_H_

#include <vector>
#include <set>

#include "llvm/IR/Instructions.h"
#include "llvm/ADT/DenseMap.h"

class CFGEdge;

class CFGNode {
public:
	enum class NodeType {
		BlockEntry,
		BlockExit,
		FuncEntry,
		FuncExit,
		Default
	};
private:
	llvm::Instruction *inst;
	int line;
	CFGNode::NodeType node_type;
	std::vector<CFGEdge *> in_edges;
	std::vector<CFGEdge *> out_edges;

public:
	CFGNode(llvm::Instruction *inst, CFGNode::NodeType node_type);
	~CFGNode();

private:
	int getLine(llvm::Instruction *inst);

public:
	llvm::Instruction *getInst();
	int getLine();
	CFGNode::NodeType getNodeType();

	void pushInEdge(CFGEdge *edge);
	void deleteInEdge(CFGEdge *edge);
	int getNumIns();
	CFGEdge *getInEdge(int in_idx);
	CFGNode *getInNode(int in_idx);

	void pushOutEdge(CFGEdge *edge);
	void deleteOutEdge(CFGEdge *edge);
	int getNumOuts();
	CFGEdge *getOutEdge(int out_idx);
	CFGNode *getOutNode(int out_idx);
};

class CFGEdge {
public:
	enum class EdgeType {
		Block,
		Branch,
		Switch,
		Call,
		Ret,
		Default
	};

private:
	CFGNode *src;
	CFGNode *dst;
	EdgeType edge_type;

public:
	CFGEdge(CFGNode *src, CFGNode *dst, CFGEdge::EdgeType edge_type);
	~CFGEdge();

public:
	CFGNode *getSrc();
	CFGNode *getDst();
	CFGEdge::EdgeType getEdgeType();
}; 

class CFG {
private:
	std::set<CFGNode *> node_set;
	std::set<CFGEdge *> edge_set;
	llvm::DenseMap<llvm::Instruction *, CFGNode *> inst2node;
	llvm::DenseMap<llvm::Function *, std::vector<CFGNode *> > func2nodes;
	std::vector<CFGNode *> entry_vec;

public:
	CFG();
	~CFG();

private:
	std::set<CFGNode *> getSuccs(CFGNode *src_node, bool is_intra);
	std::set<CFGNode *> getPreds(CFGNode *dst_node, bool is_intra);
	std::set<CFGNode *> getIntraSuccs(CFGNode *src_node);
	std::set<CFGNode *> getIntraPreds(CFGNode *dst_node);
	std::set<CFGNode *> getInterSuccs(CFGNode *src_node);
	std::set<CFGNode *> getInterPreds(CFGNode *dst_node);

protected:
	void createCFGNode(llvm::Instruction *inst, CFGNode::NodeType node_type);
	void createCFGEdge(llvm::Instruction *src_inst, llvm::Instruction *dst_inst, 
						CFGEdge::EdgeType edge_type);
	void collectEntry();
	void deleteEdge(CFGEdge *edge);
	void deleteEdge(llvm::Instruction *src_inst, llvm::Instruction *dst_inst);

public:
	std::set<CFGNode *> &getNodeSet();
	std::set<CFGEdge *> &getEdgeSet();
	CFGNode *getCFGNode(llvm::Instruction *inst);
	std::set<CFGNode *> getIntraBetween(CFGNode *src_node, CFGNode *dst_node);
	std::set<CFGNode *> getIntraBetween(llvm::Instruction *src_inst, llvm::Instruction *dst_inst);
	std::set<CFGNode *> getInterBetween(CFGNode *src_node, CFGNode *dst_node);
	std::set<CFGNode *> getInterBetween(llvm::Instruction *src_inst, llvm::Instruction *dst_inst);

	int getNumEntries();
	CFGNode *getEntry(int entry_idx);

	virtual void build() = 0;
};

#endif
