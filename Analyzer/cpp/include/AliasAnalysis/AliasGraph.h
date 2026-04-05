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
	const std::set<llvm::Value *> &getAliasSet() const;
	bool isAnchor();
	llvm::Value *getOneAnchorVal();

	void pushInEdge(AGEdge *edge);
	int getNumIns();
	AGEdge *getInEdge(int in_idx);
	AGNode *getInNode(int in_idx);

	void pushOutEdge(int offset, AGEdge *edge);
	const std::map<int, AGEdge*> &getOutEdges() const;
	AGEdge *getOutEdgeByOffset(int offset);
	AGNode *getOutNodeByOffset(int offset);

	std::string toDotNode(size_t id);
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

	std::string toDotEdge(size_t src_id, size_t dst_id);
};

struct AGPathStep {
	AGNode *node;
	AGEdge *edge;
};

class AliasGraph {
private:
	std::set<AGNode *> agnode_set;
	std::set<AGEdge *> agedge_set;
	llvm::DenseMap<llvm::Value *, AGNode *> val2node;

public:
	AliasGraph();
	virtual ~AliasGraph();

private:
	std::unordered_map<AGNode *, int> getBackDist(AGNode *dst);

protected:
	void updateAGNode(llvm::Value *val, AGNode *agnode);
	void createAGEdge(AGNode *src_node, AGNode *dst_node, int offset);

public:
	AGNode *getAGNode(llvm::Value *val);
	AGNode *findAGNode(llvm::Value *val);
	void compact(UnionFind<AGNode *> &uf);
	virtual void build() = 0;

	bool isAlias(llvm::Value *val1, llvm::Value *val2);
	AGNode *findNearestAncestor(std::vector<AGNode *> &nodes, bool should_anchor=true);
	AGNode *findNearestAncestor(std::vector<llvm::Value *> &vals, bool should_anchor=true);
	std::vector<AGPathStep> getAGPath(AGNode *src, AGNode *dst);
	std::vector<int> getOffsetAGPath(AGNode *src, AGNode *dst);

	void dumpDot(std::string dot_name);
	void dumpSvg(std::string svg_name);
};

#endif
