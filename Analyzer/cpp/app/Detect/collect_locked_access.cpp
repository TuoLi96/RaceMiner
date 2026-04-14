#include <string>
#include <sstream>
#include <vector>
#include <iostream>

#include "Manager/PathMgr/PathMgr.h"
#include "Manager/ModMgr/ModPack.h"
#include "Manager/DBMgr/DBMgr.h"
#include "Manager/DBMgr/LinkMgr.h"
#include "CFG/IntraAcycleCFG.h"
#include "Concurrency/ConcAPI/LockAPI.h"
#include "AliasAnalysis/TypeGraph.h"
#include "AliasAnalysis/Steensgaard.h"
#include "Concurrency/RaceDetector/LockCollector.h"
#include "Utils/StrOperations.h"

using namespace std;

static void analyze(PathMgr *path_mgr, ModPack *mod_pack) {
	DBMgr *db_mgr = new DBMgr(path_mgr->getRaceDBPath());
	IntraAcycleCFG *intra_acfg = new IntraAcycleCFG(mod_pack);
	intra_acfg->build();
	TypeGraph *type_graph = new TypeGraph(mod_pack);
	type_graph->analyze();
	//type_graph->dumpSvg("tg.svg");
	Steensgaard *steen = new Steensgaard(mod_pack);
	steen->build();
	//steen->dumpSvg("alias.svg");
	//steen->dumpDot("alias.dot");
	LockAPI *lock_api = new LockAPI(path_mgr, steen);
	LockCollector *lock_collector = new LockCollector(mod_pack, path_mgr, 
							intra_acfg, lock_api, steen, type_graph, db_mgr);
	lock_collector->collect();
	delete intra_acfg;
	delete db_mgr;
	delete type_graph;
	delete steen;
	delete lock_api;
	delete lock_collector;
}

int main(int argc, char *argv[]) {
	PathMgr *path_mgr = new PathMgr();
	if (argc >= 2) {
		ModPack *mod_pack = new ModPack("non.a");
		string lr_path = string(argv[1]);
		mod_pack->push(lr_path);
		analyze(path_mgr, mod_pack);
		delete mod_pack;
	} else {
		DBMgr *db_mgr = new DBMgr(path_mgr->getCompileDBPath());
		LinkMgr *link_mgr = new LinkMgr(path_mgr, db_mgr);
		vector<LinkRow> link_rows = link_mgr->selectAll();
		size_t total = link_rows.size();
		size_t link_idx = 1;
		for (auto link_row : link_rows) {
			cout << "Analyzing(" << link_idx << "/" << total << 
							"): " << link_row.target_file << endl;
			string ir_list = link_row.ir_list;
			vector<string> ir_vec = splitStr(ir_list, " ");
			ModPack *mod_pack = new ModPack(link_row.target_file);
			for (size_t ir_idx = 0; ir_idx < ir_vec.size(); ir_idx++) {
				mod_pack->push(ir_vec[ir_idx]);
			}
			analyze(path_mgr, mod_pack);
			delete mod_pack;
			link_idx++;
		}
		delete link_mgr;
		delete db_mgr;
	}
	delete path_mgr;
	return 0;
}
