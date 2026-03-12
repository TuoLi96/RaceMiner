#include "ModMgr/ModMgr.h"

#include "llvm/IRReader/IRReader.h"

using namespace std;
using namespace llvm;

ModMgr::ModMgr(string ir_path) {
	owner_ptr = parseIRFile(ir_path, err, ctx);
	if (!owner_ptr) {
		llvm::errs() << "Error parsing IR file: " << ir_path << "\n";
		err.print("ModMgr::ModMgr()", llvm::errs());
	}
	mod = owner_ptr.get();
}

ModMgr::~ModMgr() {
	// Need to do nothing at present.
}

Module *ModMgr::getMod() {
	return mod;
}
