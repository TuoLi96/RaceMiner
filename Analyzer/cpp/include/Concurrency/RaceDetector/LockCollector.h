#ifndef _LOCK_COLLECTOR_H_
#define _LOCK_COLLECTOR_H_

#include <vector>
#include <string>

#include "llvm/IR/Value.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"

#include "Manager/ModMgr/ModPack.h"
#include "AliasAnalysis/TypeGraph.h"
#include "AliasAnalysis/AliasGraph.h"
#include "CFG/IntraAcycleCFG.h"
#include "Concurrency/ConcAPI/LockAPI.h" 

class LockCollector {
private:
	ModPack *mod_pack;
	ModMgr *analyzing_mod_mgr;
	IntraAcycleCFG *cfg;
	LockAPI *lock_api;
	AliasGraph *ag;
	TypeGraph *tg;

public:
	LockCollector(ModPack *mod_pack, IntraAcycleCFG *cfg, LockAPI *lock_api,
				AliasGraph *ag, TypeGraph *tg);
	~LockCollector();

private:
	std::vector<std::string> getAccessPath(llvm::Value *lock_val, 
								llvm::Value *unlock_val, llvm::Value *access_val);
	void handleInst(llvm::CallInst *lock_inst, llvm::CallInst *unlock_inst,
								llvm::Instruction *access_inst);
	void handleFunc(llvm::Function &func);
	void handleMod(llvm::Module &mod);

public:
	void collect();
};

#endif
