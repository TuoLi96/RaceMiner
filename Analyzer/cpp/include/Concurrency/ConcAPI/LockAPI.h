#ifndef _LOCK_API_H_
#define _LOCK_API_H_

#include <map>
#include <string>

#include "llvm/IR/Instructions.h"

#include "Manager/PathMgr/PathMgr.h"
#include "AliasAnalysis/AliasGraph.h"

class LockAPI {
private:
	PathMgr *path_mgr;
	AliasGraph *ag;

	std::map<std::string, std::string> lock2unlock;
	std::map<std::string, std::string> unlock2lock;
	std::map<std::string, int> lock_val_map;
	std::map<std::string, int> unlock_val_map;

public:
	LockAPI(PathMgr *path_mgr, AliasGraph *ag);
	~LockAPI();

private:
	void readLockAPIInfo();

public:
	bool isLock(llvm::CallInst *call_inst);
	llvm::Value *getLockVal(llvm::CallInst *lock_call_inst);
	bool isUnlock(llvm::CallInst *call_inst);
	llvm::Value *getUnlockVal(llvm::CallInst *unlock_call_inst);

	bool isLockPair(llvm::CallInst *lock_call_inst1, llvm::CallInst *unlock_call_inst2);
};

#endif
