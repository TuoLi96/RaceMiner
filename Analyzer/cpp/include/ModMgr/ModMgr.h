#ifndef _MOD_MGR_H_
#define _MOD_MGR_H_

#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/SourceMgr.h"

class ModMgr {
private:
	llvm::LLVMContext ctx;
	llvm::SMDiagnostic err;
	std::unique_ptr<llvm::Module> owner_ptr;
	llvm::Module *mod;

public:
	ModMgr(std::string ir_path);
	~ModMgr();

public:
	llvm::Module *getMod();
};

#endif
