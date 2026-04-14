#include "Manager/ModMgr/ModPack.h"

using namespace std;
using namespace llvm;

ModPack::ModPack(string target_file) {
	this->target_file = target_file;
	this->mgr_vec.clear();
}

ModPack::~ModPack() {
	for (size_t mgr_i = 0; mgr_i < mgr_vec.size(); mgr_i++) {
		delete mgr_vec[mgr_i];
	}
}

string &ModPack::getTargetFile() {
	return target_file;
}

void ModPack::push(string ir_path) {
	ModMgr *mod_mgr = new ModMgr(ir_path);
	mgr_vec.push_back(mod_mgr);
}

int ModPack::getNumMgrs() {
	return (int)mgr_vec.size();
}

ModMgr *ModPack::getMgr(int mgr_idx) {
	return mgr_vec[mgr_idx];
}
