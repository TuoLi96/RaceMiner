#ifndef _RACE_DETECTOR_H_
#define _RACE_DETECTOR_H_

#include <vector>

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"

#include "Manager/PathMgr/PathMgr.h"
#include "Manager/ModMgr/ModPack.h"
#include "Manager/ModMgr/ModMgr.h"
#include "Manager/DBMgr/DBMgr.h"
#include "CFG/IntraAcycleCFG.h"
#include "CFG/AcycleCG.h"
#include "AliasAnalysis/SteensgaardInter.h"
#include "Concurrency/ConcAPI/LockAPI.h"

struct VarAccess {
	CFGNode *cfg_node;
	AGNode *ag_node;
	llvm::Value *access_val;
};

struct Summary {
	std::set<AGNode *> lock_set;
	std::set<AGNode *> unlock_set;
	std::set<VarAccess *> var_accesses;
};

class RaceDetector {
private:
	PathMgr *path_mgr;
	ModPack *mod_pack;
	ModMgr *analyzing_mod_mgr;
	DBMgr *race_db_mgr;
	IntraAcycleCFG *cfg;
	AcycleCG *cg;
	SteensgaardInter *ag;
	LockAPI *lock_api;

	llvm::DenseMap<CGNode *, int> cgnode_use_cnt;
	llvm::DenseMap<CGNode *, Summary *> cgnode_sum;
	llvm::DenseMap<AGNode *, AGNode *> lock_uses;
	llvm::DenseSet<AGNode *> used_locks;

public:
	RaceDetector(PathMgr *path_mgr, ModPack *mod_pack, DBMgr *race_db_mgr,
					IntraAcycleCFG *cfg, AcycleCG *cg, SteensgaardInter *ag);
	~RaceDetector();

private:
	void readLockUses();
	bool isValidSummary(Summary *sum);
	Summary *mergeSummaries(std::vector<Summary *> &sum_vec);
	void dropSummary(Summary *sum);
	Summary *useFuncSummary(llvm::CallInst *call_inst, Summary *sum);
	Summary *analyzeCallInst(llvm::CallInst *call_inst, Summary *sum);
	Summary *analyzeCFGNode(CFGNode *cfg_node, Summary *sum);
	Summary *analyzeFunc(llvm::Function *func);
	void record(Summary *sum);

public:
	void detect();
};

#endif
