#ifndef _LOCK_COLLECTOR_H_
#define _LOCK_COLLECTOR_H_

#include <vector>
#include <string>

#include "llvm/IR/Value.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"

#include "Manager/ModMgr/ModPack.h"
#include "Manager/DBMgr/LockCollectionMgr.h"
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

	LockCollectionMgr *lock_collection_mgr;

public:
	LockCollector(ModPack *mod_pack, PathMgr *path_mgr,
				IntraAcycleCFG *cfg, LockAPI *lock_api,
				AliasGraph *ag, TypeGraph *tg,
				DBMgr *db_mgr);
	~LockCollector();

private:
	void record(llvm::CallInst *lock_call, llvm::CallInst *unlock_call, 
					llvm::Instruction *access_inst, std::string lock_var,
					std::string unlock_var, std::string access_var);
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
