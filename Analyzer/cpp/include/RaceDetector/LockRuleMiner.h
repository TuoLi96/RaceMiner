#ifndef _LOCK_RULE_MINER_H_
#define _LOCK_RULE_MINER_H_

#include "ModMgr/ModPack.h"

class LockRuleMiner {
private:
	ModPack *mod_pack;
	ModMgr *analyzing_mod_mgr;

public:
	LockRuleMiner(ModPack *mod_pack);
	~LockRuleMiner();

public:
	void analyze();
};

#endif
