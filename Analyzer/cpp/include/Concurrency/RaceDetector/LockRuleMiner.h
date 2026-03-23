#ifndef _LOCK_RULE_MINER_H_
#define _LOCK_RULE_MINER_H_

#include "ModMgr/ModPack.h"
#include "AliasAnalysis/TypeGraph.h"
#include "CFG/CFG.h"
#include "CFG/IntraCFG.h"

class LockRuleMiner {
private:
	ModPack *mod_pack;
	ModMgr *analyzing_mod_mgr;
	IntraCFG *cfg;

	std::vector<std::string> lock_func_vec;
	std::vector<int> lock_arg_idx_vec;
	std::vector<std::string> unlock_func_vec;
	std::vector<int> unlock_arg_idx_vec;

public:
	LockRuleMiner(ModPack *mod_pack, IntraCFG *cfg, LockAPI *lock_api);
	~LockRuleMiner();

public:
	void pushLockFuncInfo(std::string lock_func, int lock_arg_idx,
						std::string unlock_func, int unlock_arg_idx);
	void analyze();
};

#endif
