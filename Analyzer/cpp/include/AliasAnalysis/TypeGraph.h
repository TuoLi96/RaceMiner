#ifndef _TYPE_GRAPH_H_
#define _TYPE_GRAPH_H_

#include <set>
#include <string>
#include <map>
#include <unordered_map>

#include "llvm/IR/DebugInfoMetadata.h"

#include "ModMgr/ModMgr.h"
#include "ModMgr/ModPack.h"

class TypeEdge;

class TypeNode {
private:
	std::string type_name;
	std::vector<std::string> aliased_type_names;
	std::set<llvm::Value *> val_set;
	std::map<int, TypeEdge *> in_offset_idx;
	std::map<std::string, TypeEdge *> in_field_idx;
	std::map<int, TypeEdge *> out_offset_idx;
	std::map<std::string, TypeEdge *> out_field_idx;
public:
	TypeNode(std::string type_name);
	~TypeNode();

public:
	void pushTypeName(std::string type_name);
	void insertVal(llvm::Value *val);
	void addInEdge(int offset, std::string field, TypeEdge *edge);
	void addOutEdge(int offset, std::string field, TypeEdge *edge);

	std::string toDotNode(size_t id);
};

class TypeEdge {
private:
	TypeNode *src;
	TypeNode *dst;
	int offset;
	std::string field;
public:
	TypeEdge(TypeNode *src, TypeNode *dst, int offset, std::string field);
	~TypeEdge();

public:
	TypeNode *getSrc();
	TypeNode *getDst();
	int getOffset();
	std::string getField();

	std::string toDotEdge(size_t src_id, size_t dst_id);
};

class TypeGraph {
private:
	ModPack *mod_pack;
	std::set<TypeNode *> tynode_set;
	std::set<TypeEdge *> tyedge_set;
	std::unordered_map<llvm::DIType *, TypeNode *> ditype2node;
	std::unordered_map<llvm::Value *, TypeNode *> val2node;

public:
	TypeGraph(ModPack *mod_pack);
	~TypeGraph();

	void dumpDot(std::string dot_name);
	void dumpSvg(std::string svg_name);

private:
	TypeNode *getOrCreateTypeNode(llvm::DIType *ditype);
	TypeEdge *createTypeEdge(TypeNode *src, TypeNode *dst, int offset, std::string field);
	void handleDIElements(TypeNode *base_tynode, llvm::DINodeArray &elem_array);
	void handleDICompositeType(llvm::DICompositeType *composite_ditype);
	void handleDIPointerType(llvm::DIDerivedType *pointer_ditype);
	void handleDITypedef(llvm::DIDerivedType *typedef_ditype);
	void handleDIDerivedType(llvm::DIDerivedType *derived_ditype);
	void handleDIBasicType(llvm::DIBasicType *basic_ditype);
	void handleDIType(llvm::DIType *ditype);
	void handleMod(llvm::Module *mod);

public:
	void analyze();
};

#endif
