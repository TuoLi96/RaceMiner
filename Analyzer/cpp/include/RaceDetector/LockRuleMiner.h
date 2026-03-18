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

public:
	LockRuleMiner(ModPack *mod_pack);
	~LockRuleMiner();

public:
	void analyze();
};

#endif
