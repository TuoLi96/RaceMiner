#include "Manager/ModMgr/ModMgr.h"
#include "Utils/IROperations.h"

#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/BinaryFormat/Dwarf.h"

#include <iostream>

using namespace std;
using namespace llvm;

ModMgr::ModMgr(string ir_path) {
	owner_ptr = parseIRFile(ir_path, err, ctx);
	if (!owner_ptr) {
		llvm::errs() << "Error parsing IR file: " << ir_path << "\n";
		err.print("ModMgr::ModMgr()", llvm::errs());
	}
	mod = owner_ptr.get();
	mst = new ModuleSlotTracker(mod);
	collectDIType();
}

ModMgr::~ModMgr() {
	delete mst;
}

void ModMgr::collectDIType() {
	// TODO: Handle bit-field
	for (GlobalVariable &GV : mod->globals()) {
		auto *dbg_var = getDbgVar(&GV);
		if (dbg_var) {
			DIType *ditype = stripDITypedef(dbg_var->getType());
			if (ditype) {
				val2ditype[&GV] = ditype;
			}
		}
	}
	
	for (auto &func : *mod) {
		for (auto &blk : func) {
			for (auto &inst : blk) {
				if (AllocaInst *alloca_inst = dyn_cast<AllocaInst>(&inst)) {
					auto *dbg_var = getDbgVar(alloca_inst);
					if (dbg_var) {
						DIType *ditype = stripDITypedef(dbg_var->getType());
						if (ditype) {
							val2ditype[alloca_inst] = ditype;
						}
					}
				} else if (LoadInst *load_inst = dyn_cast<LoadInst>(&inst)) {
					Value *pointer = load_inst->getPointerOperand();
					auto type_find = val2ditype.find(pointer);
					if (type_find != val2ditype.end()) {
						DIType *ditype = type_find->second;
						DIDerivedType *di_derive = dyn_cast<DIDerivedType>(ditype);
						if (di_derive && di_derive->getTag() == dwarf::Tag::DW_TAG_pointer_type) {
							DIType *base_ditype = stripDITypedef(di_derive->getBaseType());
							if (base_ditype) {
								val2ditype[load_inst] = base_ditype;
							}	
						}
					}
				} else if (GetElementPtrInst *gep_inst = dyn_cast<GetElementPtrInst>(&inst)) {
					int64_t offset = getStructOffset(gep_inst);
					Value *pointer = gep_inst->getPointerOperand();
					auto type_find = val2ditype.find(pointer);
					if (offset != INT64_MIN && type_find != val2ditype.end()) {
						DIType *ditype = type_find->second;
						DICompositeType *di_composite = dyn_cast<DICompositeType>(ditype);
						if (di_composite && di_composite->getTag() == dwarf::Tag::DW_TAG_structure_type) {
							auto elem_array = di_composite->getElements();
							for (auto elem : elem_array) {
								DIDerivedType *di_derive = dyn_cast<DIDerivedType>(elem);
								uint64_t offset_bits = di_derive->getOffsetInBits();
								// FIXME: Can offset be negative
								if (offset * 8 == offset_bits) {
									if (!(di_derive->getFlags() & DINode::FlagBitField)) {
										DIType *base_ditype = stripDITypedef(di_derive->getBaseType());
										if (base_ditype) {
											val2ditype[gep_inst] = base_ditype;
										}
										gep2field[gep_inst] = di_derive->getName().str();
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

Module *ModMgr::getMod() {
	return mod;
}

string ModMgr::getFieldOfGep(GetElementPtrInst *gep_inst) {
	auto field_find = gep2field.find(gep_inst);
	if (field_find != gep2field.end()) {
		return field_find->second;
	} else {
		return "";
	}
}

pair<Value *, string> ModMgr::getAnchorFieldPath(Value *val) {
	vector<GetElementPtrInst *> gep_vec;
	string field_path = "";
	while (true) {
		if (LoadInst *load_inst = dyn_cast<LoadInst>(val)) {
			val = load_inst->getOperand(0);
		} else if (CastInst *cast_inst = dyn_cast<CastInst>(val)) {
			val = cast_inst->getOperand(0);
		} else if (GetElementPtrInst *gep_inst = dyn_cast<GetElementPtrInst>(val)) {
			val = gep_inst->getPointerOperand();
			gep_vec.push_back(gep_inst);
		} else {
			break;
		}
	}
	if (!isa<AllocaInst>(val) && !isa<GlobalVariable>(val)) {
		return {NULL, ""};
	}
	for (int gep_idx = (int)(gep_vec.size()) - 1; gep_idx >= 0; gep_idx--) {
		auto field_find = gep2field.find(gep_vec[gep_idx]);
		if (field_find == gep2field.end()) {
			return {NULL, ""};
		}
		string field = field_find->second;
		field_path += ".";
		field_path += field;
	}
	return {val, field_path};
}

string ModMgr::getRawMetadata(Metadata *metadata) {
	string meta_str;
	llvm::raw_string_ostream raw_meta_str(meta_str);
	metadata->print(raw_meta_str, *mst);
	raw_meta_str.flush();
	return meta_str;
}

string ModMgr::getRawVal(Value *val) {
	string val_str;
	llvm::raw_string_ostream raw_val_str(val_str);
	val->print(raw_val_str, *mst);
	raw_val_str.flush();
	return val_str;
}
