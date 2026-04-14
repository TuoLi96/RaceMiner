#include <string>
#include <vector>
#include <iostream>
#include <map>

#include "Manager/PathMgr/PathMgr.h"
#include "Manager/ModMgr/ModPack.h"
#include "Manager/DBMgr/DBMgr.h"
#include "Manager/DBMgr/LinkMgr.h"
#include "Manager/DBMgr/LockUseMgr.h"
#include "Manager/DBMgr/LockCollectionMgr.h"
#include "CFG/IntraAcycleCFG.h"
#include "Concurrency/ConcAPI/LockAPI.h"
#include "AliasAnalysis/TypeGraph.h"
#include "AliasAnalysis/Steensgaard.h"
#include "Concurrency/RaceDetector/LockCollector.h"
#include "Utils/StrOperations.h"

using namespace std;

static void collect(PathMgr *path_mgr, LockCollectionMgr *lock_collection_mgr,
						LockUseMgr *lock_use_mgr, string target_file, vector<string> &src_vec) {
	map<string, map<string, int>> access_lock_var_stat;

	for (size_t src_idx = 0; src_idx < src_vec.size(); src_idx++) {
		vector<LockCollectionRow> rows = lock_collection_mgr->selectFile(src_vec[src_idx]);
		for (auto &row : rows) {
			access_lock_var_stat[row.lock_var][row.access_var]++;
		}
	}
	for (auto &access_lock_var : access_lock_var_stat) {
		const string &lock_var = access_lock_var.first;
		for (auto &access_var : access_lock_var.second) {
			LockUseRow row = {target_file, lock_var, access_var.first, access_var.second};
			lock_use_mgr->insert(row);
		}
	}
}

int main(int argc, char *argv[]) {
	PathMgr *path_mgr = new PathMgr();
	DBMgr *race_db_mgr = new DBMgr(path_mgr->getRaceDBPath());
	LockCollectionMgr *lock_collection_mgr = new LockCollectionMgr(path_mgr, race_db_mgr);
	LockUseMgr *lock_use_mgr = new LockUseMgr(path_mgr, race_db_mgr);
	if (argc >= 2) {
		string src_path = string(argv[1]);
		vector<string> src_vec = {src_path};
		collect(path_mgr, lock_collection_mgr, lock_use_mgr, "non.a", src_vec);
	} else {
		DBMgr *db_mgr = new DBMgr(path_mgr->getCompileDBPath());
		LinkMgr *link_mgr = new LinkMgr(path_mgr, db_mgr);
		vector<LinkRow> link_rows = link_mgr->selectAll();
		size_t total = link_rows.size();
		size_t link_idx = 1;
		for (auto link_row : link_rows) {
			cout << "Analyzing(" << link_idx << "/" << total << 
							"): " << link_row.target_file << endl;
			string src_list = link_row.link_list;
			vector<string> src_vec = splitStr(src_list, " ");
			collect(path_mgr, lock_collection_mgr, lock_use_mgr, link_row.target_file, src_vec);
			link_idx++;
		}
		delete link_mgr;
		delete db_mgr;
	}
	delete lock_use_mgr;
	delete lock_collection_mgr;
	delete race_db_mgr;
	delete path_mgr;
	return 0;
}
