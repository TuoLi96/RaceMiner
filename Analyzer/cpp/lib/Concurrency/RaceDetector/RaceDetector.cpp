#include "Concurrency/RaceDetector/RaceDetector.h"

#include "Manager/DBMgr/LockUseMgr.h"

#include <iostream>

using namespace std;
using namespace llvm;

/*
 * Implementation of RaceDetector
 */

RaceDetector::RaceDetector(PathMgr *path_mgr, 
					ModPack *mod_pack, DBMgr *race_db_mgr, 
					IntraAcycleCFG *cfg, AcycleCG *cg, SteensgaardInter *ag) {
	this->path_mgr = path_mgr;
	this->mod_pack = mod_pack;
	this->analyzing_mod_mgr = NULL;
	this->race_db_mgr = race_db_mgr;
	this->cfg = cfg;
	this->cg = cg;
	this->ag = ag;
	this->cgnode_use_cnt.clear();
	this->cgnode_sum.clear();
	this->lock_uses.clear();
	this->used_locks.clear();
	this->lock_api = new LockAPI(path_mgr, ag);
	readLockUses();
}

RaceDetector::~RaceDetector() {
	for (auto &cgnode_sum_it : cgnode_sum) {
		dropSummary(cgnode_sum_it.second);
	}
	cgnode_sum.clear();
}

void RaceDetector::readLockUses() {
	LockUseMgr *lock_use_mgr = new LockUseMgr(path_mgr, race_db_mgr);
	vector<LockUseRow> lock_use_rows = lock_use_mgr->selectTarget(mod_pack->getTargetFile());
	for (size_t row_idx = 0; row_idx < lock_use_rows.size(); row_idx++) {
		LockUseRow &row = lock_use_rows[row_idx];
		string lock_var = row.lock_var;
		string access_var = row.access_var;
		size_t pos = lock_var.find('.');
		string ancestor_type = lock_var.substr(0, pos);
		string lock_field_path = lock_var.substr(pos + 1);
		pos = access_var.find('.');
		string access_field_path = access_var.substr(pos + 1);
		set<AGNode *> type_nodes = ag->getNodesOfType(ancestor_type);
		for (auto *type_node : type_nodes) {
			AGNode *lock_agnode = ag->getNodeOfFieldPath(type_node, lock_field_path);
			AGNode *access_agnode = ag->getNodeOfFieldPath(type_node, access_field_path);
			if (lock_agnode && access_agnode) {
				lock_uses[access_agnode] = lock_agnode;
				used_locks.insert(lock_agnode);
			}
		}
	}
}

bool RaceDetector::isValidSummary(Summary *sum) {
	if (sum == NULL) {
		return false;
	}
	if (!sum->lock_set.empty() || 
			!sum->unlock_set.empty() || 
			!sum->var_accesses.empty()) {
		return true;
	}
	return false;
}

Summary *RaceDetector::mergeSummaries(vector<Summary *> &sum_vec) {
	if (sum_vec.size() == 0) {
		return NULL;
	}
	Summary *new_sum = new Summary();
	new_sum->lock_set = sum_vec[0]->lock_set;
	for (size_t sum_idx = 1; sum_idx < sum_vec.size(); sum_idx++) {
		set<AGNode *> tmp;
		for (auto *agnode : new_sum->lock_set) {
			if (sum_vec[sum_idx]->lock_set.count(agnode)) {
				tmp.insert(agnode);
			}
		}
		new_sum->lock_set.swap(tmp);
	}
	for (auto *sum : sum_vec) {
		new_sum->unlock_set.insert(sum->unlock_set.begin(), sum->unlock_set.end());
	}
	for (auto *sum : sum_vec) {
		for (auto *var_access : sum->var_accesses) {
			VarAccess *new_var_access = new VarAccess();
			new_var_access->cfg_node = var_access->cfg_node;
			new_var_access->ag_node = var_access->ag_node;
			new_var_access->access_val = var_access->access_val;
			new_sum->var_accesses.insert(new_var_access);
		}
	}
	return new_sum;
}

void RaceDetector::dropSummary(Summary *sum) {
	if (!sum) {
		return;
	}
	for (auto &var_access : sum->var_accesses) {
		delete var_access;
	}
	delete sum;
}

Summary *RaceDetector::useFuncSummary(CallInst *call_inst, Summary *sum) {
	CGEdge *cgedge = cg->getCGEdge(call_inst);
	if (cgedge == NULL) {
		return NULL;
	}
	CGNode *cgnode = cgedge->getDst();
	auto callee_sum_find = cgnode_sum.find(cgnode);
	if (callee_sum_find == cgnode_sum.end()) {
		return NULL;
	}
	Summary *callee_sum = callee_sum_find->second;
	cgnode_use_cnt[cgnode]--;
	if (cgnode_use_cnt[cgnode] == 0 && sum == NULL) {
		sum = callee_sum;
		cgnode_sum.erase(cgnode);
		cgnode_use_cnt.erase(cgnode);
	} else {
		if (sum == NULL) {
			sum = new Summary();
		}
		for (auto unlock_agnode : callee_sum->unlock_set) {
			if (sum->lock_set.find(unlock_agnode) != sum->lock_set.end()) {
				sum->lock_set.erase(unlock_agnode);
			} else {
				for (auto var_it = sum->var_accesses.begin(); 
							var_it != sum->var_accesses.end(); ) {
					AGNode *ag_node = (*var_it)->ag_node;
					auto lock_find = lock_uses.find(ag_node);
					if (lock_find != lock_uses.end() &&
								lock_find->second == unlock_agnode) {
						var_it = sum->var_accesses.erase(var_it);
					} else {
						++var_it;
					}
				}
			}
		}
		for (auto var_access : callee_sum->var_accesses) {
			auto lock_find = lock_uses.find(var_access->ag_node);
			if (lock_find != lock_uses.end()) {
				if (sum->lock_set.find(lock_find->second) == sum->lock_set.end()) {
					VarAccess *new_var_access = new VarAccess();
					new_var_access->cfg_node = var_access->cfg_node;
					new_var_access->ag_node = var_access->ag_node;
					new_var_access->access_val = var_access->access_val;
					sum->var_accesses.insert(new_var_access);
				}
			}
		}
		for (auto lock_node : callee_sum->lock_set) {
			sum->lock_set.insert(lock_node);
		}
	}
	return sum;
}


Summary *RaceDetector::analyzeCallInst(CallInst *call_inst, Summary *sum) {
	if (lock_api->isLock(call_inst)) {
		Value *lock_val = lock_api->getLockVal(call_inst);
		AGNode *lock_agnode = ag->findAGNode(lock_val);
		if (lock_agnode && used_locks.count(lock_agnode)) {
			if (!sum) {
				sum = new Summary();
			}
			sum->lock_set.insert(lock_agnode);
		}
	} else if (lock_api->isUnlock(call_inst)) {
		Value *unlock_val = lock_api->getUnlockVal(call_inst);
		AGNode *unlock_agnode = ag->findAGNode(unlock_val);
		if (unlock_agnode) {
			if (!sum) {
				sum = new Summary();
			}
			if (sum->lock_set.find(unlock_agnode) != sum->lock_set.end()) {
				sum->lock_set.erase(unlock_agnode);
			} else {
				sum->unlock_set.insert(unlock_agnode);
				for (auto var_it = sum->var_accesses.begin(); 
							var_it != sum->var_accesses.end(); ) {
					AGNode *ag_node = (*var_it)->ag_node;
					auto lock_find = lock_uses.find(ag_node);
					if (lock_find != lock_uses.end() &&
								lock_find->second == unlock_agnode) {
						var_it = sum->var_accesses.erase(var_it);
					} else {
						++var_it;
					}
				}
			}
		}
	} else {
		sum = useFuncSummary(call_inst, sum);
	}
	return sum;
}

Summary *RaceDetector::analyzeCFGNode(CFGNode *cfg_node, Summary *sum) {
	Instruction *inst = cfg_node->getInst();
	if (LoadInst *load_inst = dyn_cast<LoadInst>(inst)) {
		AGNode *agnode = ag->findAGNode(load_inst);
		auto lock_find = lock_uses.find(agnode);
		if (lock_find != lock_uses.end()) {
			if (sum == NULL) {
				sum = new Summary();
			}
			if (sum->lock_set.find(lock_find->second) == sum->lock_set.end()) {
				VarAccess *var_access = new VarAccess();
				var_access->cfg_node = cfg_node;
				var_access->ag_node = agnode;
				var_access->access_val = load_inst;
				sum->var_accesses.insert(var_access);
			}
		}
	} else if (CallInst *call_inst = dyn_cast<CallInst>(inst)) {
		sum = analyzeCallInst(call_inst, sum);
	}
	return sum;
}

Summary *RaceDetector::analyzeFunc(Function *func) {
	vector<CFGNode *> &topo_vec = cfg->getTopoVecOfFunc(func);
	llvm::DenseMap<CFGNode *, Summary *> cfgnode2sum;
	llvm::DenseMap<CFGNode *, int> cfgnode_use_cnt;
	Summary *ret_sum = NULL;
	for (size_t node_idx = 0; node_idx < topo_vec.size(); node_idx++) {
		CFGNode *cfgnode = topo_vec[node_idx];
		Summary *sum = NULL;
		vector<Summary *> in_sum_vec;
		vector<bool> own_vec;
		for (int pre_idx = 0; pre_idx < cfgnode->getNumIns(); pre_idx++) {
			CFGNode *pre_cfgnode = cfgnode->getInNode(pre_idx);
			auto sum_find = cfgnode2sum.find(pre_cfgnode);
			if (sum_find != cfgnode2sum.end()) {
				in_sum_vec.push_back(sum_find->second);
				cfgnode_use_cnt[pre_cfgnode]--;
				if (cfgnode_use_cnt[pre_cfgnode] == 0) {
					cfgnode2sum.erase(pre_cfgnode);
					cfgnode_use_cnt.erase(pre_cfgnode);
					own_vec.push_back(true);
				} else {
					own_vec.push_back(false);
				}
			}
		}
		int in_sum_size = in_sum_vec.size();
		if (in_sum_size == 1 && own_vec[0]) {
			sum = in_sum_vec[0];
		} else {
			sum = mergeSummaries(in_sum_vec);
			for (size_t own_idx = 0; own_idx < own_vec.size(); own_idx++) {
				if (own_vec[own_idx]) {
					dropSummary(in_sum_vec[own_idx]);
					in_sum_vec[own_idx] = NULL;
				}
			}
		}
		sum = analyzeCFGNode(cfgnode, sum);
		if (isValidSummary(sum)) {
			if (cfgnode->getNumOuts() != 0) {
				cfgnode2sum[cfgnode] = sum;
				cfgnode_use_cnt[cfgnode] = cfgnode->getNumOuts();
			} else if (isa<ReturnInst>(cfgnode->getInst()) && ret_sum == NULL) {
				ret_sum = sum;
			} else {
				dropSummary(sum);
			}
		} else {
			dropSummary(sum);
		}
	}
	return ret_sum;
}

void RaceDetector::record(Summary *sum) {
	for (auto var_access : sum->var_accesses) {
		CFGNode *cfg_node = var_access->cfg_node;
		Value *access_val = var_access->access_val;
		cout << "11111111111" << endl;
	}
}

void RaceDetector::detect() {
	for (int func_idx = cg->getNumTopoNodes() - 1; func_idx >= 0; func_idx--) {
		CGNode *cgnode = cg->getTopoNode(func_idx);
		Function *func = cgnode->getFunc();
		Summary *sum = analyzeFunc(func);
		if (isValidSummary(sum)) {
			int in_num = cgnode->getNumIns();
			if (in_num != 0) {
				cgnode_sum[cgnode] = sum;
				cgnode_use_cnt[cgnode] = in_num;
			} else {
				record(sum);
				dropSummary(sum);
			}
		} else {
			dropSummary(sum);
		}
	}
}
