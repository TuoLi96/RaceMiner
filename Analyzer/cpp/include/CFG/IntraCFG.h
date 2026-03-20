#ifndef _INTRA_CFG_H_
#define _INTRA_CFG_H_

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"

#include "CFG/CFG.h"
#include "Manager/ModMgr/ModMgr.h"
#include "Manager/ModMgr/ModPack.h"

class IntraCFG : CFG {
private:
	ModPack *mod_pack;
	ModMgr *analyzing_mod_mgr;

public:
	IntraCFG(ModPack *mod_pack);
	~IntraCFG();

private:
	void createNodeForBlock(llvm::BasicBlock &blk);
	void createNodeForFunc(llvm::Function &func);
	void createNodeForMod(llvm::Module &mod);
	void createNodeForPack();

	void createEdgeForBlock(llvm::BasicBlock &blk);
	void createEdgeForFunc(llvm::Function &func);
	void createEdgeForMod(llvm::Module &mod);
	void createEdgeForPack();

public:
	void build();
};

#endif
