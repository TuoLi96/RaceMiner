#ifndef _ALIAS_GRAPH_H_
#define _ALIAS_GRAPH_H_

#include <vector>
#include <set>
#include <map>

#include "llvm/IR/Value.h"
#include "llvm/ADT/DenseMap.h"

#include "Utils/UnionFind.h"

class AGEdge;

class AGNode {
private:
	std::set<llvm::Value *> alias_set;
	std::vector<AGEdge *> in_edges;
	std::map<int, AGEdge *> out_edges;

public:
	AGNode();
	~AGNode();

public:
	void insertVal(llvm::Value *val);

	void pushInEdge(AGEdge *edge);
	int getNumIns();
	AGEdge *getInEdge(int in_idx);
	AGNode *getInNode(int in_idx);

	void pushOutEdge(int offset, AGEdge *edge);
	const std::map<int, AGEdge*>& getOutEdges() const;
	AGEdge *getOutEdgeByOffset(int offset);
	AGNode *getOutNodeByOffset(int offset);
};

class AGEdge {
private:
	AGNode *src;
	AGNode *dst;
	int offset;

public:
	AGEdge(AGNode *src, AGNode *dst, int offset);
	~AGEdge();

public:
	AGNode *getSrc();
	AGNode *getDst();
	int getOffset();
};

class AliasGraph {
private:
	std::set<AGNode *> agnode_set;
	std::set<AGEdge *> agedge_set;
	llvm::DenseMap<llvm::Value *, AGNode *> val2node;

public:
	AliasGraph();
	~AliasGraph();

protected:
	void createAGNode(llvm::Value *val);
	void createAGEdge(AGNode *src_node, AGNode *dst_node, int offset);

public:
	AGNode *getAGNode(llvm::Value *val);
	void compact(UnionFind<AGNode *> &uf);
	virtual void build() = 0;
};

#endif
