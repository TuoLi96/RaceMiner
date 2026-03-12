#include "AliasAnalysis/TypeGraph.h"
#include "Constants.h"

#include <spdlog/spdlog.h>

#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/BinaryFormat/Dwarf.h"

#include <fstream>

using namespace std;
using namespace llvm;

/* 
 * Implementation of TypeNode
 */

TypeNode::TypeNode(string type_name) {
	this->type_name = type_name;
	this->aliased_type_names.clear();
	this->val_set.clear();
	this->in_offset_idx.clear();
	this->in_field_idx.clear();
	this->out_offset_idx.clear();
	this->out_field_idx.clear();
}

TypeNode::~TypeNode() {
	// Need to do nothing at present.
}

void TypeNode::pushTypeName(string type_name) {
	this->aliased_type_names.push_back(type_name);
}

void TypeNode::insertVal(Value *val) {
	this->val_set.insert(val);
}

void TypeNode::addInEdge(int offset, string field, TypeEdge *edge) {
	in_offset_idx[offset] = edge;
	in_field_idx[field] = edge;
}

void TypeNode::addOutEdge(int offset, string field, TypeEdge *edge) {
	out_offset_idx[offset] = edge;
	out_field_idx[field] = edge;
}

std::string TypeNode::toDotNode(size_t id) {
	string node_str;
	node_str += "node" + std::to_string(id);
	node_str += " [shape=plaintext label=<";
	node_str += "<TABLE BORDER=\"1\" CELLBORDER=\"0\" CELLSPACING=\"0\">";

	// type_name
	node_str += "<TR><TD BGCOLOR=\"lightgray\">";
	node_str += type_name;
	node_str += "</TD></TR>";

	// blank line
    node_str += "<TR><TD></TD></TR>";

	// alias type names
	for (auto &alias : aliased_type_names) {
		node_str += "<TR><TD ALIGN=\"LEFT\">";
		node_str += alias;
		node_str += "</TD></TR>";
	}

	node_str += "</TABLE>";
	node_str += ">];";

	return node_str;
}

/* 
 * Implementation of TypeEdge
 */

TypeEdge::TypeEdge(TypeNode *src, TypeNode *dst, int offset, string field) {
	this->src = src;
	this->dst = dst;
	this->offset = offset;
	this->field = field;
}

TypeEdge::~TypeEdge() {
	// Need to do nothing at present.
}

TypeNode *TypeEdge::getSrc() {
	return this->src;
}

TypeNode *TypeEdge::getDst() {
	return this->dst;
}

int TypeEdge::getOffset() {
	return this->offset;
}

string TypeEdge::getField() {
	return this->field;
}

string TypeEdge::toDotEdge(size_t src_id, size_t dst_id) {
	string edge_str;

	edge_str += "node" + to_string(src_id);
	edge_str += " -> ";
	edge_str += "node" + to_string(dst_id);

	edge_str += " [label=\"";
	edge_str += to_string(offset);
	edge_str += " : ";
	edge_str += field;
	edge_str += "\"]";

	edge_str += ";";

	return edge_str;
}

/* 
 * Implementation of TypeGraph
 */

TypeGraph::TypeGraph(ModPack *mod_pack) {
	this->mod_pack = mod_pack;
	this->tynode_set.clear();
	this->tyedge_set.clear();
	this->ditype2node.clear();
	this->val2node.clear();
}

TypeGraph::~TypeGraph() {
	for (auto tynode : this->tynode_set) {
		delete tynode;
	}
	for (auto tyedge : this->tyedge_set) {
		delete tyedge;
	}
}

TypeNode *TypeGraph::getOrCreateTypeNode(DIType *ditype) {
	auto ditype_find = ditype2node.find(ditype);
	if (ditype_find != ditype2node.end()) {
		return ditype_find->second;
	} else {
		string name = "@Non";
		auto name_ref = ditype->getName();
		if (!name_ref.empty()) {
			name = name_ref;
		} else {
			DIType *tmp_ditype = ditype;
			string tmp_name = "";
			while (true) {
				DIDerivedType *derived_ditype = dyn_cast<DIDerivedType>(tmp_ditype);
				if (derived_ditype == NULL) {
					auto name_ref = tmp_ditype->getName();
					if (!name_ref.empty()) {
						tmp_name = tmp_name + name_ref.str();
					} else {
						tmp_name = "";
					}
					break;
				}
				auto tag = derived_ditype->getTag();
				if (tag == dwarf::Tag::DW_TAG_pointer_type) {
					tmp_name = tmp_name + "*";
					tmp_ditype = derived_ditype->getBaseType();
				} else {
					auto name_ref = tmp_ditype->getName();
					if (!name_ref.empty()) {
						tmp_name = tmp_name + name_ref.str();
					} else {
						tmp_name = "";
					}
					break;
				}
			}
			if (tmp_name != "") {
				name = tmp_name;
			}
		}
		TypeNode *new_tynode= new TypeNode(name);
		tynode_set.insert(new_tynode);
		ditype2node[ditype] = new_tynode;
		return new_tynode;
	}
}

TypeEdge *TypeGraph::createTypeEdge(TypeNode *src, TypeNode *dst, int offset, string field) {
	TypeEdge *new_tyedge = new TypeEdge(src, dst, offset, field);
	src->addOutEdge(offset, field, new_tyedge);
	dst->addInEdge(offset, field, new_tyedge);
	tyedge_set.insert(new_tyedge);
	return new_tyedge;
}

void TypeGraph::handleDIElements(TypeNode *base_tynode, DINodeArray &elem_array) {
	int offset = 0;
	for (auto elem : elem_array) {
		DIDerivedType *derived_ditype = dyn_cast<DIDerivedType>(elem);
		auto elem_tag = derived_ditype->getTag();
		if (derived_ditype == NULL) {
			spdlog::error("TypeGraph::handleDIElements: Not derived type {}",
						dwarf::TagString(elem_tag));
			break;
		}
		if (elem_tag != dwarf::Tag::DW_TAG_member) {
			spdlog::error("TypeGraph::handleDIElements: TAG mismatch {}",
						dwarf::TagString(elem_tag));
			break;
		}
		string field = derived_ditype->getName().str();
		DIType *elem_ditype = derived_ditype->getBaseType();
		TypeNode *elem_tynode = getOrCreateTypeNode(elem_ditype);
		createTypeEdge(base_tynode, elem_tynode, offset, field);
		offset++;
	}
}

void TypeGraph::handleDICompositeType(DICompositeType *composite_ditype) {	
	auto composite_tag = composite_ditype->getTag();
	TypeNode *base_tynode = getOrCreateTypeNode(composite_ditype);
	auto elem_array = composite_ditype->getElements();

	switch (composite_tag) {
		case dwarf::Tag::DW_TAG_structure_type:
			handleDIElements(base_tynode, elem_array);
			break;
		case dwarf::Tag::DW_TAG_union_type:
			// TODO: Handle Union
			break;
		default:
			spdlog::warn("TypeGraph::handleDICompositeType: Unhandled DW_TAG {}", 
						dwarf::TagString(composite_tag));			
			break;
	}
	 
}

void TypeGraph::handleDIPointerType(DIDerivedType *pointer_ditype) {
	TypeNode *pointer_tynode = getOrCreateTypeNode(pointer_ditype);
	TypeNode *val_tynode = getOrCreateTypeNode(pointer_ditype->getBaseType());
	int offset = REF_OFFSET;
	string field = "*";
	TypeEdge *tyedge = createTypeEdge(pointer_tynode, val_tynode, offset, field);
}

void TypeGraph::handleDITypedef(DIDerivedType *typedef_ditype) {
	TypeNode *base_tynode = getOrCreateTypeNode(typedef_ditype->getBaseType());
	string type_name = typedef_ditype->getName().str();
	base_tynode->pushTypeName(type_name);
	ditype2node[typedef_ditype] = base_tynode;
}

void TypeGraph::handleDIDerivedType(DIDerivedType *derived_ditype) {
	auto derived_tag = derived_ditype->getTag();
	TypeNode *base_tynode = getOrCreateTypeNode(derived_ditype);
	switch (derived_tag) {
		case dwarf::Tag::DW_TAG_member:
			break;
		case dwarf::Tag::DW_TAG_pointer_type:
			handleDIPointerType(derived_ditype);
			break;
		case dwarf::Tag::DW_TAG_typedef:
			handleDITypedef(derived_ditype);
			break;
		default:
			spdlog::warn("TypeGraph::handleDIDerivedType: Unhandled DW_TAG {}",
						dwarf::TagString(derived_tag));
	}
}

void TypeGraph::handleDIBasicType(DIBasicType *basic_ditype) {
	TypeNode *basic_tynode = getOrCreateTypeNode(basic_ditype);
}

void TypeGraph::handleDIType(DIType *ditype) {
	if (DICompositeType *compose_ditype = dyn_cast<DICompositeType>(ditype)) {
		handleDICompositeType(compose_ditype);
	} else if (DIDerivedType *derived_ditype = dyn_cast<DIDerivedType>(ditype)) {
		handleDIDerivedType(derived_ditype);
	} else if (DIBasicType *basic_ditype = dyn_cast<DIBasicType>(ditype)) {
		handleDIBasicType(basic_ditype);
	}
}

void TypeGraph::handleMod(Module *mod) {
	DebugInfoFinder finder;
	finder.processModule(*mod);
	for (auto ditype : finder.types()) {
		handleDIType(ditype);
	}
}

void TypeGraph::analyze() {
	for (size_t mod_i = 0 ; mod_i < mod_pack->getNumMgrs(); mod_i++) {
		ModMgr *mod_mgr = mod_pack->getMgr(mod_i);
		Module *mod = mod_mgr->getMod();
		handleMod(mod);
	}
}

void TypeGraph::dumpDot(string dot_file) {
	ofstream ofs(dot_file);

	ofs << "digraph TypeGraph {\n";

	ofs << "rankdir=LR;\n";
	ofs << "splines=true;\n";
	ofs << "overlap=false;\n";
	ofs << "nodesep=0.6;\n";
	ofs << "ranksep=1.0;\n";

	unordered_map<TypeNode*, size_t> tynode_id;

	size_t id = 0;

	for (auto tynode : tynode_set) {
		tynode_id[tynode] = id;
		ofs << tynode->toDotNode(id) << "\n";
		id++;
	}

	for (auto tyedge : tyedge_set) {
		TypeNode *src = tyedge->getSrc();
		TypeNode *dst = tyedge->getDst();

		ofs << tyedge->toDotEdge(
			tynode_id[src],
			tynode_id[dst]
		) << "\n";
	}

    ofs << "}\n";

	ofs.close();
}

void TypeGraph::dumpSvg(std::string svg_name) {
	string dot = "typegraph.dot";

	dumpDot(dot);

	std::string cmd = "dot -Tsvg " + dot + " -o " + svg_name;

	system(cmd.c_str());
}
