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
#include "Utils/IROperations.h"

#include "llvm/IR/Instructions.h"

using namespace std;
using namespace llvm;

static void analyze(PathMgr *path_mgr, ModPack *mod_pack) {
	TypeGraph *type_graph = new TypeGraph(mod_pack);
	type_graph->analyze();
	for (int mod_idx = 0; mod_idx < mod_pack->getNumMgrs(); mod_idx++) {
		ModMgr *mod_mgr = mod_pack->getMgr(mod_idx);
		Module *mod = mod_mgr->getMod();
		for (auto &func : *mod) {
			for (auto &blk : func) {
				for (auto &inst : blk) {
					if (auto *store_inst = dyn_cast<StoreInst>(&inst)) {
						auto *ptr = store_inst->getPointerOperand();
						auto *val = store_inst->getValueOperand();

						auto *gep = dyn_cast<GetElementPtrInst>(ptr);
						if (!gep) {
							continue;
						}
						cout << val2str(gep) << endl;
						uint64_t offset = type_graph->getStructOffset(gep);
						cout << offset << endl;
						BitRange br = type_graph->parseBitRange(val, offset);
						cout << br.bit_offset << "  " << br.width << "  " << br.valid << endl;
					}
				}
			}
		}
	}
	delete type_graph;
}

int main(int argc, char *argv[]) {
	PathMgr *path_mgr = new PathMgr();
	if (argc >= 2) {
		ModPack *mod_pack = new ModPack();
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
			//cout << "Analyzing(" << link_idx << "/" << total << 
			//				"): " << link_row.target_file << endl;
			/*if (link_row.target_file.find("lib/tests") == string::npos) {
				continue;
			}*/
			string ir_list = link_row.ir_list;
			istringstream iss(ir_list);
			vector<string> ir_vec;
			string ir_file;
			while (iss >> ir_file) {
				ir_vec.push_back(ir_file);
			}
			ModPack *mod_pack = new ModPack();
			for (auto ir_file : ir_vec) {
				if (ir_file.find("crypto/keyring.ll") == string::npos) {
					continue;
				}
				cout << ir_file << endl;
				mod_pack->push(ir_file);
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
