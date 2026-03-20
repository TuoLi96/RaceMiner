#include "Concurrency/ConcAPI/LockAPI.h"

#include <fstream>

using namespace std;
using namespace llvm;

/*
 * Implementation of LockAPI
 */

LockAPI::LockAPI(PathMgr *path_mgr, AliasGraph *ag) {
	this->path_mgr = path_mgr;
	this->ag = ag;
	this->lock2unlock.clear();
	this->unlock2lock.clear();
	this->lock_val_map.clear();
	this->unlock_val_map.clear();
}

LockAPI::~LockAPI() {
	// Need to do nothing at present.
}

void LockAPI::readLockAPIInfo() {
	string lock_api_path = path_mgr->getAPILockPath();
	ifstream lock_api_ifs(lock_api_path);
	string lock_api_line;
	while (getline(lock_api_ifs, lock_api_line)) {
		stringstream lock_api_ss(lock_api_line);
		string lock_func, unlock_func;
		int lock_val, unlock_val;
		if (lock_api_ss >> lock_func >> lock_val >> unlock_func >> unlock_val) {
			lock2unlock[lock_func] = unlock_func;
			unlock2lock[unlock_func] = lock_func;
			lock_val_map[lock_func] = lock_val;
			unlock_val_map[unlock_func] = unlock_val;
		}
	}
}

bool LockAPI::isLock(CallInst *call_inst) {
	Function *func = call_inst->getCalledFunction();
	if (func) {
		string func_name = func->getName().str();
		if (lock2unlock.find(func_name) != lock2unlock.end()) {
			return true;
		}
	}
	return false;
}

Value *LockAPI::getLockVal(CallInst *lock_call_inst) {
	Function *func = lock_call_inst->getCalledFunction();
	string func_name = func->getName().str();
	Value *lock_val = lock_call_inst->getArgOperand(lock_val_map[func_name]);
	return lock_val;
}

bool LockAPI::isUnlock(CallInst *call_inst) {
	Function *func = call_inst->getCalledFunction();
	if (func) {
		string func_name = func->getName().str();
		if (unlock2lock.find(func_name) != unlock2lock.end()) {
			return true;
		}
	}
	return false;
}

Value *LockAPI::getUnlockVal(CallInst *unlock_call_inst) {
	Function *func = unlock_call_inst->getCalledFunction();
	string func_name = func->getName().str();
	Value *unlock_val = unlock_call_inst->getArgOperand(unlock_val_map[func_name]);
	return unlock_val;
}

bool LockAPI::isLockPair(CallInst *call_inst1, CallInst *call_inst2) {
	if (!isLock(call_inst1) || !isUnlock(call_inst2)) {
		return false;
	}
	Value *lock_val = getLockVal(call_inst1);
	Value *unlock_val = getUnlockVal(call_inst2);
	if (ag->isAlias(lock_val, unlock_val)) {
		return true;
	}
	return true;
}
