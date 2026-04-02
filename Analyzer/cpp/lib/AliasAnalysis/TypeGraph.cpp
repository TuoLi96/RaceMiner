#include "AliasAnalysis/TypeGraph.h"
#include "Utils/IROperations.h"
#include "Constants.h"

#include <spdlog/spdlog.h>

#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"

#include <fstream>
#include <iostream>

using namespace std;
using namespace llvm;

/* 
 * Implementation of TypeNode
 */

TypeNode::TypeNode(string type_name) {
	this->node_type = TypeNode::NodeType::Default;
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

void TypeNode::setNodeType(TypeNode::NodeType node_type) {
	this->node_type = node_type;
}

void TypeNode::updateTypeName(string new_type_name) {
	this->type_name = new_type_name;
}

string TypeNode::getTypeName() {
	if (type_name != "") {
		return type_name;
	} else if (!aliased_type_names.empty()) {
		// Note: Return the first aliased type name.
		return aliased_type_names[0];
	} else {
		return "@Non";
	}
}

bool TypeNode::isBasic() {
	return this->node_type == TypeNode::NodeType::Basic;
}

bool TypeNode::isComposite() {
	return this->node_type == TypeNode::NodeType::Composite;
}

bool TypeNode::isDerived() {
	return this->node_type == TypeNode::NodeType::Derived;
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

TypeEdge *TypeNode::getOutEdgeByOffset(int offset) {
	auto out_find = out_offset_idx.find(offset);
	if (out_find == out_offset_idx.end()) {
		return NULL;
	} else {
		return out_find->second;
	}
}

TypeNode *TypeNode::getOutNodeByOffset(int offset) {
	TypeEdge *out_edge = getOutEdgeByOffset(offset);
	if (out_edge) {
		return out_edge->getDst();
	} else {
		return NULL;
	}
}

TypeEdge *TypeNode::getOutEdgeByField(string field) {
	auto out_find = out_field_idx.find(field);
	if (out_find == out_field_idx.end()) {
		return NULL;
	} else {
		return out_find->second;
	}
}

TypeNode *TypeNode::getOutNodeByField(string field) {
	TypeEdge *out_edge = getOutEdgeByField(field);
	if (out_edge) {
		return out_edge->getDst();
	} else {
		return NULL;
	}
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
	string offset_str;
	if (offset == REF_OFFSET) {
		offset_str = "ref";
	} else if (offset == GEP_OFFSET) {
		offset_str = "gep";
	} else {
		offset_str = to_string(offset);
	}

	edge_str += "node" + to_string(src_id);
	edge_str += " -> ";
	edge_str += "node" + to_string(dst_id);

	edge_str += " [label=\"";
	edge_str += offset_str;
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

string TypeGraph::getTagStr(dwarf::Tag ditype_tag) {
	return dwarf::TagString(ditype_tag).str();
}

string TypeGraph::getTagStr(DIType *ditype) {
	auto ditype_tag = ditype->getTag();
	return dwarf::TagString(ditype_tag).str();
}

TypeNode *TypeGraph::createTypeNode(DIType *ditype) {
	string name = "";
	auto name_ref = ditype->getName();
	if (!name_ref.empty()) {
		name = name_ref;
	} else {
		DIType *tmp_ditype = ditype;
		string tmp_name = "";
		while (true) {
			if (tmp_ditype == NULL) {
				tmp_name = "";
				break;
			}
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
	if (isa<DIBasicType>(ditype)) {
		new_tynode->setNodeType(TypeNode::NodeType::Basic);
	}
	tynode_set.insert(new_tynode);
	ditype2node[ditype] = new_tynode;
	return new_tynode;
}

TypeNode *TypeGraph::getTypeNode(DIType *ditype) {
	auto tynode_finder = ditype2node.find(ditype);
	if (tynode_finder != ditype2node.end()) {
		return tynode_finder->second;
	} else {
		spdlog::error("TypeGraph::getTypeNode: No TypeNode found for Tag {}",
					getTagStr(ditype));
		return NULL;
	}
}

void TypeGraph::createTypeEdge(TypeNode *src, TypeNode *dst, int offset, string field) {
	if (dst->isBasic()) {
		//return;
	}
	TypeEdge *new_tyedge = new TypeEdge(src, dst, offset, field);
	src->addOutEdge(offset, field, new_tyedge);
	dst->addInEdge(offset, field, new_tyedge);
	tyedge_set.insert(new_tyedge);
}

void TypeGraph::handleDIElements(TypeNode *base_tynode, DINodeArray &elem_array) {
	int offset = 0;
	for (auto elem : elem_array) {
		DIDerivedType *derived_ditype = dyn_cast<DIDerivedType>(elem);
		auto elem_tag = derived_ditype->getTag();
		if (derived_ditype == NULL) {
			spdlog::error("TypeGraph::handleDIElements: Not derived type {}",
						getTagStr(elem_tag));
			break;
		}
		if (elem_tag != dwarf::Tag::DW_TAG_member) {
			spdlog::error("TypeGraph::handleDIElements: TAG mismatch {}",
						getTagStr(elem_tag));
			break;
		}
		string field = derived_ditype->getName().str();
		DIType *elem_ditype = derived_ditype->getBaseType();
		TypeNode *elem_tynode = getTypeNode(elem_ditype);
		createTypeEdge(base_tynode, elem_tynode, offset, field);
		offset++;
	}
}

void TypeGraph::handleDICompositeType(DICompositeType *composite_ditype) {	
	auto composite_tag = composite_ditype->getTag();
	TypeNode *base_tynode = getTypeNode(composite_ditype);
	auto elem_array = composite_ditype->getElements();

	switch (composite_tag) {
		case dwarf::Tag::DW_TAG_const_type:
		case dwarf::Tag::DW_TAG_array_type:
		case dwarf::Tag::DW_TAG_volatile_type:
		case dwarf::Tag::DW_TAG_enumeration_type:
			break;
		case dwarf::Tag::DW_TAG_structure_type:
		case dwarf::Tag::DW_TAG_union_type:
			handleDIElements(base_tynode, elem_array);
			break;
		default:
			spdlog::warn("TypeGraph::handleDICompositeType: Unhandled DW_TAG {}", 
						getTagStr(composite_tag));			
			break;
	}
	 
}

void TypeGraph::handleDIPointerType(DIDerivedType *pointer_ditype) {
	TypeNode *pointer_tynode = getTypeNode(pointer_ditype);
	if (pointer_ditype->getBaseType() == NULL) {
		// TODO: Handle baseType: null;
		pointer_tynode->setNodeType(TypeNode::NodeType::Basic);
		pointer_tynode->updateTypeName("void");
		return;
	}
	TypeNode *val_tynode = getTypeNode(pointer_ditype->getBaseType());
	int offset = REF_OFFSET;
	string field = "*";
	createTypeEdge(pointer_tynode, val_tynode, offset, field);
}

void TypeGraph::handleDITypedef(DIDerivedType *typedef_ditype) {
	DIType *base_ditype = typedef_ditype->getBaseType();
	if (!base_ditype) {
		createTypeNode(typedef_ditype);
		return;
	}
	if (base_ditype->getTag() == dwarf::Tag::DW_TAG_typedef) {
		handleDITypedef(dyn_cast<DIDerivedType>(base_ditype));
	}
	TypeNode *base_tynode = getTypeNode(base_ditype);
	string type_name = typedef_ditype->getName().str();
	base_tynode->pushTypeName(type_name);
	ditype2node[typedef_ditype] = base_tynode;
}

void TypeGraph::handleDIDerivedType(DIDerivedType *derived_ditype) {
	auto derived_tag = derived_ditype->getTag();
	switch (derived_tag) {
		case dwarf::Tag::DW_TAG_member:
		case dwarf::Tag::DW_TAG_typedef:
		case dwarf::Tag::DW_TAG_const_type:
		case dwarf::Tag::DW_TAG_array_type:
		case dwarf::Tag::DW_TAG_volatile_type:
			break;
		case dwarf::Tag::DW_TAG_pointer_type:
			handleDIPointerType(derived_ditype);
			break;
		default:
			spdlog::warn("TypeGraph::handleDIDerivedType: Unhandled DW_TAG {}",
						getTagStr(derived_tag));
	}
}

void TypeGraph::handleDISubroutineType(DISubroutineType *subroutine_ditype) {
	auto arg_array = subroutine_ditype->getTypeArray();
	string sub_type_name = "";
	TypeNode *sub_tynode = getTypeNode(subroutine_ditype);
	int arg_idx = -1;
	for (auto arg : arg_array) {
		string type_name;
		if (arg == NULL) {
			type_name = "void";
		} else {
			TypeNode *arg_tynode = getTypeNode(arg);
			type_name = arg_tynode->getTypeName();
		}
		if (arg_idx == -1) {
			sub_type_name = type_name + "(";
		} else if (arg_idx == 0) {
			sub_type_name = sub_type_name + type_name;
		} else {
			sub_type_name = sub_type_name + ", " + type_name;
		}
		arg_idx++;
	}
	sub_type_name = sub_type_name + ")";
	sub_tynode->updateTypeName(sub_type_name);
}

void TypeGraph::handleDIBasicType(DIBasicType *basic_ditype) {
	getTypeNode(basic_ditype);
}

void TypeGraph::handleDIType(DIType *ditype) {
	if (DICompositeType *compose_ditype = dyn_cast<DICompositeType>(ditype)) {
		handleDICompositeType(compose_ditype);
	} else if (DIDerivedType *derived_ditype = dyn_cast<DIDerivedType>(ditype)) {
		handleDIDerivedType(derived_ditype);
	} else if (DISubroutineType *subroutine_ditype = dyn_cast<DISubroutineType>(ditype)) {
		handleDISubroutineType(subroutine_ditype);
	} else if (DIBasicType *basic_ditype = dyn_cast<DIBasicType>(ditype)) {
		handleDIBasicType(basic_ditype);
	}
}

void TypeGraph::handleMod(Module *mod) {
	DebugInfoFinder finder;
	finder.processModule(*mod);
	for (auto ditype : finder.types()) {
		auto ditype_tag = ditype->getTag();
		switch (ditype_tag) {
			case dwarf::Tag::DW_TAG_member:
			case dwarf::Tag::DW_TAG_typedef:
				break;
			case dwarf::Tag::DW_TAG_pointer_type:
			case dwarf::Tag::DW_TAG_structure_type:
			case dwarf::Tag::DW_TAG_base_type:
			case dwarf::Tag::DW_TAG_union_type:
			case dwarf::Tag::DW_TAG_array_type:
			case dwarf::Tag::DW_TAG_const_type:
			case dwarf::Tag::DW_TAG_enumeration_type:
			case dwarf::Tag::DW_TAG_volatile_type:
			case dwarf::Tag::DW_TAG_subroutine_type:
			case dwarf::Tag::DW_TAG_restrict_type:
				createTypeNode(ditype);
				break;
			default:
				spdlog::warn("TypeGraph::handleMod: Unhandled DW_TAG {}",
						getTagStr(ditype_tag));
				break;
		}
	}
	for (auto ditype : finder.types()) {
		auto ditype_tag = ditype->getTag();
		if (ditype_tag == dwarf::Tag::DW_TAG_typedef) {
			handleDITypedef(dyn_cast<DIDerivedType>(ditype));	
		}
	}
	for (auto ditype : finder.types()) {
		handleDIType(ditype);
	}
}

string TypeGraph::getFieldPath(Value *val, vector<int> &offset_vec) {
	string field_path = "";
	DIVariable *dbg_var = getDbgVar(val);
	if (!dbg_var) {
		return "";
	}
	DIType *ditype = dbg_var->getType();
	TypeNode *type_node = getTypeNode(ditype);
	for (size_t offset_idx = 0; offset_idx < offset_vec.size(); offset_idx++) {
		int offset = offset_vec[offset_idx];
		TypeEdge *type_edge = type_node->getOutEdgeByOffset(offset);
		if (type_edge == NULL) {
			return "";
		}
		type_node = type_edge->getDst();
		string field_name = type_edge->getField();
		if (field_name == "*") {
			continue;
		} else {
			field_path = field_path + "." + field_name;
		}
	}
	return field_path;
}

void TypeGraph::analyze() {
	for (int mod_i = 0 ; mod_i < mod_pack->getNumMgrs(); mod_i++) {
		analyzing_mod_mgr = mod_pack->getMgr(mod_i);
		Module *mod = analyzing_mod_mgr->getMod();
		handleMod(mod);
	}
}

string TypeGraph::getTypePath(Value *val, vector<int> &offset_vec) {
	DIVariable *dbg_var = getDbgVar(val);
	if (!dbg_var) {
		return "";
	}
	DIType *ditype = dbg_var->getType();
	TypeNode *tynode = getTypeNode(ditype);
	string type_name = tynode->getTypeName();
	if (!type_name.empty() && type_name[0] == '*') {
		type_name.erase(0, 1);
	}
	return type_name + getFieldPath(val, offset_vec);
}

string TypeGraph::getNamePath(Value *val, vector<int> &offset_vec) {
	DIVariable *dbg_var = getDbgVar(val);
	if (!dbg_var) {
		return "";
	}
	return dbg_var->getName().str() + getFieldPath(val, offset_vec);	
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
		if (tynode->isBasic()) {
			//continue;
		}
		tynode_id[tynode] = id;
		ofs << tynode->toDotNode(id) << "\n";
		id++;
	}

	for (auto tyedge : tyedge_set) {
		TypeNode *src = tyedge->getSrc();
		TypeNode *dst = tyedge->getDst();
		if (dst->isBasic()) {
			//continue;
		}

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
