#ifndef _LOCK_COLLECTOR_H_
#define _LOCK_COLLECTOR_H_

#include "ModMgr/ModPack.h"
#include "AliasAnalysis/TypeGraph.h"
#include "CFG/IntraAcycleCFG.h"
#include "Concurrency/ConcAPI/LockAPI.h" 

class LockCollector {
private:
	ModPack *mod_pack;
	ModMgr *analyzing_mod_mgr;
	IntraAcycleCFG *cfg;
	LockAPI *lock_api;
	AliasGraph *ag;

public:
	LockCollector(ModPack *mod_pack, IntraCFG *cfg, LockAPI *lock_api);
	~LockCollector();

public:
	void collect();
};

#endif
