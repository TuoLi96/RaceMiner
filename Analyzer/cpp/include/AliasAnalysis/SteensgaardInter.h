#ifndef _STEENSGAARD_INTER_H_
#define _STEENSGAARD_INTER_H_

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

#include "Manager/ModMgr/ModPack.h"
#include "AliasAnalysis/AliasGraph.h"
#include "AliasAnalysis/Steensgaard.h"
#include "Utils/UnionFind.h"

class SteensgaardInter : public Steensgaard {
private:
	ModPack *mod_pack;
	ModMgr *analyzing_mod_mgr;

protected:
	UnionFind<AGNode *> uf;

public:
	SteensgaardInter(ModPack *mod_pack);
	~SteensgaardInter();

private:
	void handleCall(llvm::CallInst *call_inst);
	void handleBlock(llvm::BasicBlock &blk);
	void handleFunc(llvm::Function &func);
	void handleMod(llvm::Module &mod);

protected:
	void handlePack();

public:
	void build();
};

#endif
