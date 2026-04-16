#ifndef _ACYCLE_CG_H_
#define _ACYCLE_CG_H_

#include <vector>
#include <set>
#include <string>

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringMap.h"

#include "Manager/ModMgr/ModPack.h"
#include "CFG/CG.h"

class AcycleCG : public CG {
private:
	ModPack *mod_pack;
	ModMgr *analyzing_mod_mgr;
	std::vector<CGNode *> topo_vec;

public:
	AcycleCG(ModPack *mod_pack);
	~AcycleCG();

private:
	void breakCycle();
	void topoSort();

public:
	void build();
	int getNumTopoNodes();
	CGNode *getTopoNode(int topo_idx);
};

#endif
