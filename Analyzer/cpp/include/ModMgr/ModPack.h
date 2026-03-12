#ifndef _MOD_PACK_H_
#define _MOD_PACK_H_

#include <vector>

#include "ModMgr.h"

class ModPack {
private:
	std::vector<ModMgr *> mgr_vec;

public:
	ModPack();
	~ModPack();

public:
	void push(std::string ir_path);
	int getNumMgrs();
	ModMgr *getMgr(int mgr_idx);
};

#endif
