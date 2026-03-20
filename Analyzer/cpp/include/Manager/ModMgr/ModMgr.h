#ifndef _MOD_MGR_H_
#define _MOD_MGR_H_

#include "llvm/IR/Value.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/ModuleSlotTracker.h"
#include "llvm/IR/DebugInfoMetadata.h"

class ModMgr {
private:
	llvm::LLVMContext ctx;
	llvm::SMDiagnostic err;
	std::unique_ptr<llvm::Module> owner_ptr;
	llvm::Module *mod;
	llvm::ModuleSlotTracker *mst;

public:
	ModMgr(std::string ir_path);
	~ModMgr();

public:
	llvm::Module *getMod();
	std::string getRawMetadata(llvm::Metadata *metadata);
	std::string getRawVal(llvm::Value *val);
};

#endif
