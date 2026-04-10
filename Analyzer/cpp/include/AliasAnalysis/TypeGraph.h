#ifndef _TYPE_GRAPH_H_
#define _TYPE_GRAPH_H_

#include <set>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>

#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/BinaryFormat/Dwarf.h"
#include "llvm/IR/Instructions.h"

#include "Manager/ModMgr/ModMgr.h"
#include "Manager/ModMgr/ModPack.h"

struct BitRange {
	uint64_t bit_offset;
	uint64_t width;
	bool valid;
	BitRange() : bit_offset(0), width(0), valid(false) {}
};

class TypeEdge;

class TypeNode {
public:
	enum class NodeType {Basic, Composite, Derived, Default};
private:
	NodeType node_type;
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
	void setNodeType(NodeType node_type);
	void updateTypeName(std::string new_type_name);
	std::string getTypeName();
	bool isBasic();
	bool isComposite();
	bool isDerived();
	void pushTypeName(std::string type_name);
	void insertVal(llvm::Value *val);

	// FIXME: Multiple edges can have identical offset and field.
	void addInEdge(int offset, std::string field, TypeEdge *edge);
	void addOutEdge(int offset, std::string field, TypeEdge *edge);
	TypeEdge *getOutEdgeByOffset(int offset);
	TypeNode *getOutNodeByOffset(int offset);
	TypeEdge *getOutEdgeByField(std::string field);
	TypeNode *getOutNodeByField(std::string field);

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
	ModMgr *analyzing_mod_mgr;
	std::set<TypeNode *> tynode_set;
	std::set<TypeEdge *> tyedge_set;
	std::unordered_map<llvm::DIType *, TypeNode *> ditype2node;
	std::unordered_map<llvm::Value *, TypeNode *> val2node;

public:
	TypeGraph(ModPack *mod_pack);
	~TypeGraph();

private:
	std::string getTagStr(llvm::dwarf::Tag ditype_tag);		// Debug at present.
	std::string getTagStr(llvm::DIType *type);				// Debug at present
	TypeNode *createTypeNode(llvm::DIType *ditype);
	TypeNode *getTypeNode(llvm::DIType *ditype);
	void createTypeEdge(TypeNode *src, TypeNode *dst, int offset, std::string field);
	void handleDIElements(TypeNode *base_tynode, llvm::DINodeArray &elem_array);
	void handleDICompositeType(llvm::DICompositeType *composite_ditype);
	void handleDIPointerType(llvm::DIDerivedType *pointer_ditype);
	void handleDITypedef(llvm::DIDerivedType *typedef_ditype);
	void handleDIDerivedType(llvm::DIDerivedType *derived_ditype);
	void handleDISubroutineType(llvm::DISubroutineType *subroutine_ditype);
	void handleDIBasicType(llvm::DIBasicType *basic_ditype);
	void handleDIType(llvm::DIType *ditype);
	void handleMod(llvm::Module *mod);
	
	std::string getFieldPath(llvm::Value *val, std::vector<int> &offset);

public: // Temp for debug
	BitRange parseBitRange(llvm::Value *val, uint64_t base_byte_offset);
	int64_t getStructOffset(llvm::GetElementPtrInst *gep);

public:
	void analyze();
	std::string getTypePath(llvm::Value *val, std::vector<int> &offset);
	std::string getNamePath(llvm::Value *val, std::vector<int> &offset);
	void dumpDot(std::string dot_name);
	void dumpSvg(std::string svg_name);
};

#endif
