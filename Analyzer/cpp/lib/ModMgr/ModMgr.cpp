#include "ModMgr/ModMgr.h"

#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/raw_ostream.h"

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
}

ModMgr::~ModMgr() {
	delete mst;
}

Module *ModMgr::getMod() {
	return mod;
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
