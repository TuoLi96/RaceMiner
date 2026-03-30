#include <string>

#include "Manager/PathMgr/PathMgr.h"
#include "Manager/ModMgr/ModPack.h"
#include "CFG/IntraAcycleCFG.h"
#include "Concurrency/ConcAPI/LockAPI.h"
#include "AliasAnalysis/TypeGraph.h"
#include "AliasAnalysis/Steensgaard.h"
#include "Concurrency/RaceDetector/LockCollector.h"

using namespace std;

int main(int argc, char *argv[]) {
	PathMgr *path_mgr = new PathMgr();
	ModPack *mod_pack = new ModPack();
	if (argc >= 2) {
		string lr_path = string(argv[1]);
		mod_pack->push(lr_path);
	}
	DBMgr *db_mgr = new DBMgr(path_mgr->getRaceDBPath());
	IntraAcycleCFG *intra_acfg = new IntraAcycleCFG(mod_pack);
	intra_acfg->build();
	TypeGraph *type_graph = new TypeGraph(mod_pack);
	type_graph->analyze();
	type_graph->dumpSvg("test.svg");
	Steensgaard *steen = new Steensgaard(mod_pack);
	steen->build();
	LockAPI *lock_api = new LockAPI(path_mgr, steen);
	LockCollector *lock_collector = new LockCollector(mod_pack, path_mgr, 
							intra_acfg, lock_api, steen, type_graph, db_mgr);
	lock_collector->collect();
	delete path_mgr;
	delete mod_pack;
	delete db_mgr;
	delete type_graph;
	delete steen;
	delete lock_api;
	delete lock_collector;
	return 0;
}
