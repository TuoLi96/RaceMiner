#ifndef _CG_H_
#define _CG_H_

#include <vector>
#include <set>
#include <string>

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringMap.h"

#include "Manager/ModMgr/ModPack.h"

class CGEdge;

class CGNode {
private:
	llvm::Function *func;
	std::vector<CGEdge *> in_edges;
	std::vector<CGEdge *> out_edges;

public:
	CGNode(llvm::Function *func);
	~CGNode();

public:
	llvm::Function *getFunc();

	void pushInEdge(CGEdge *edge);
	void deleteInEdge(CGEdge *edge);
	int getNumIns();
	CGEdge *getInEdge(int in_idx);
	CGNode *getInNode(int in_idx);

	void pushOutEdge(CGEdge *edge);
	void deleteOutEdge(CGEdge *edge);
	int getNumOuts();
	CGEdge *getOutEdge(int out_idx);
	CGNode *getOutNode(int out_idx);
};

class CGEdge {
public:
	enum class EdgeType {
		Direct,
		Indirect
	};

private:
	CGNode *src;
	CGNode *dst;
	llvm::CallInst *call_inst;
	EdgeType edge_type;

public:
	CGEdge(CGNode *src, CGNode *dst, llvm::CallInst *call_inst, CGEdge::EdgeType edge_type);
	~CGEdge();

public:
	CGNode *getSrc();
	CGNode *getDst();
	llvm::CallInst *getCallInst();
	CGEdge::EdgeType getEdgeType();
};

class CG {
private:
	ModPack *mod_pack;
	ModMgr *analyzing_mod_mgr;
	std::set<CGNode *> node_set;
	std::set<CGEdge *> edge_set;
	llvm::DenseMap<llvm::Function *, CGNode *> func2node;
	llvm::DenseMap<llvm::CallInst *, CGEdge *> call2edge;
	llvm::StringMap<llvm::Function *> name2func;
	std::vector<CGNode *> entry_vec;
	std::set<CGNode *> entry_set;

private:
	void createCGNode(llvm::Function *func);
	void createCGEdge(llvm::Function *src_func, llvm::Function *dst_func,
						llvm::CallInst *call_inst, CGEdge::EdgeType edge_type);
	void collectEntry();
	void deleteEdge(CGEdge *edge);
	void deleteEdge(llvm::Function *src_func, llvm::Function *dst_func);

	llvm::Function *getFuncByName(std::string func_name);

	void createNodeForMod(llvm::Module &mod);
	void createNodeForPack();
	void createEdgeForCallInst(llvm::CallInst *call_inst);
	void createEdgeForBlock(llvm::BasicBlock &blk);
	void createEdgeForFunc(llvm::Function &func);
	void createEdgeForMod(llvm::Module &mod);
	void createEdgeForPack();

public:
	CG(ModPack *mod_pack);
	~CG();

public:
	void build();

	CGNode *getCGNode(llvm::Function *func);
	CGEdge *getCGEdge(llvm::CallInst *call_inst);
	llvm::Function *getCallee(llvm::CallInst *call_inst);

	int getNumEntries(); 
	CGNode *getEntry(int entry_idx);
};

#endif
