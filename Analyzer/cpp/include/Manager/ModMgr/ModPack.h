#ifndef _MOD_PACK_H_
#define _MOD_PACK_H_

#include <vector>
#include <string>

#include "ModMgr.h"

class ModPack {
private:
	std::string target_file;
	std::vector<ModMgr *> mgr_vec;

public:
	ModPack(std::string target_file);
	~ModPack();

public:
	std::string &getTargetFile();
	void push(std::string ir_path);
	int getNumMgrs();
	ModMgr *getMgr(int mgr_idx);
};

#endif
