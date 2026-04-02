#ifndef _INTRA_ACYCLE_CFG_H_
#define _INTRA_ACYCLE_CFG_H_

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/DenseMap.h"

#include "CFG/CFG.h"
#include "CFG/IntraCFG.h"
#include "Manager/ModMgr/ModPack.h"

class IntraAcycleCFG : public IntraCFG {
private:
	ModPack *mod_pack;
	ModMgr *analyzing_mod_mgr;
	llvm::DenseMap<llvm::Function *, std::vector<CFGNode *> > func2topo_vec;
	std::vector<CFGNode *> topo_vec;

public:
	IntraAcycleCFG(ModPack *mod_pack);
	~IntraAcycleCFG();

private:
	void breakRemainCycle();
	void breakCycleForFunc(llvm::Function &func);
	void breakCycleForMod(llvm::Module &mod);
	void breakCycle();
	void topoSort();

public:
	void build();
	int getNumTopoNodes();
	CFGNode *getTopoNode(int topo_idx);

	std::vector<CFGNode *> &getTopoVecOfFunc(llvm::Function *func);
};

#endif
