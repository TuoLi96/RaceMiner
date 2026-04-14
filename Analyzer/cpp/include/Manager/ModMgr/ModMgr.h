#ifndef _MOD_MGR_H_
#define _MOD_MGR_H_

#include "llvm/IR/Value.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/ModuleSlotTracker.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugInfo.h"

class ModMgr {
private:
	std::string ir_path;
	std::string src_path;
	llvm::LLVMContext ctx;
	llvm::SMDiagnostic err;
	std::unique_ptr<llvm::Module> owner_ptr;
	llvm::Module *mod;
	llvm::ModuleSlotTracker *mst;

	llvm::DenseMap<llvm::Value *, llvm::DIType *> val2ditype;
	llvm::DenseMap<llvm::GetElementPtrInst *, std::string> gep2field;

public:
	ModMgr(std::string ir_path);
	~ModMgr();

private:
	void collectDIType();

public:
	std::string &getIrPath();
	std::string &getSrcPath();
	llvm::Module *getMod();
	std::string getFieldOfGep(llvm::GetElementPtrInst *gep_inst);
	std::pair<llvm::Value *, std::string> getAnchorFieldPath(llvm::Value *val);
	std::string getRawMetadata(llvm::Metadata *metadata);
	std::string getRawVal(llvm::Value *val);
};

#endif
