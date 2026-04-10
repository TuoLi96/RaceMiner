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
	llvm::Value *anchor_val;
	std::string anchor_name;
	std::string type_name;

	std::set<llvm::Value *> alias_set;
	std::vector<AGEdge *> in_edges;
	std::map<std::string, AGEdge *> out_edges;

public:
	AGNode();
	~AGNode();

public:
	void setAnchor(llvm::Value *anchor_val, std::string anchor_name);
	bool isAnchor();
	llvm::Value *getAnchorVal();
	std::string getAnchorName();

	void setTypeName(std::string type_name);
	std::string getTypeName();

	void insertVal(llvm::Value *val);
	const std::set<llvm::Value *> &getAliasSet() const;

	void pushInEdge(AGEdge *edge);
	int getNumIns();
	AGEdge *getInEdge(int in_idx);
	AGNode *getInNode(int in_idx);

	void pushOutEdge(std::string field, AGEdge *edge);
	const std::map<std::string, AGEdge*> &getOutEdges() const;
	AGEdge *getOutEdge(std::string field);
	AGNode *getOutNode(std::string field);

	std::string toDotNode(size_t id);
};

class AGEdge {
private:
	AGNode *src;
	AGNode *dst;
	std::string field;

public:
	AGEdge(AGNode *src, AGNode *dst, std::string field);
	~AGEdge();

public:
	AGNode *getSrc();
	AGNode *getDst();
	std::string getField();

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
	void createAGEdge(AGNode *src_node, AGNode *dst_node, std::string field);

public:
	AGNode *getAGNode(llvm::Value *val);
	AGNode *findAGNode(llvm::Value *val);
	void compact(UnionFind<AGNode *> &uf);
	virtual void build() = 0;

	bool isAlias(llvm::Value *val1, llvm::Value *val2);
	AGNode *findNearestAncestor(std::vector<AGNode *> &nodes, bool should_anchor=true);
	AGNode *findNearestAncestor(std::vector<llvm::Value *> &vals, bool should_anchor=true);
	std::vector<AGEdge *> getFieldPathEdges(AGNode *src, AGNode *dst);
	std::string getFieldPath(AGNode *src, AGNode *dst);

	void dumpDot(std::string dot_name);
	void dumpSvg(std::string svg_name);
};

#endif
