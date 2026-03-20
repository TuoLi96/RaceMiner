#ifndef _STEENSGAARD_H_
#define _STEENSGAARD_H_

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

#include "Manager/ModMgr/ModPack.h"
#include "AliasAnalysis/AliasGraph.h"
#include "Utils/UnionFind.h"

class Steensgaard : public AliasGraph {
private:
	ModPack *mod_pack;
	ModMgr *analyzing_mod_mgr;
	UnionFind<AGNode *> uf;

public:
	Steensgaard(ModPack *mod_pack);
	~Steensgaard();

private:
	void unify(AGNode *agnode1, AGNode *agnode2);

	void createNodeForInst(llvm::Instruction *inst);
	void createNodeForBlock(llvm::BasicBlock &blk);
	void createNodeForFunc(llvm::Function &func);
	void createNodeForMod(llvm::Module &mod);
	void createNodeForPack();

	void handleLoad(llvm::LoadInst *load_inst);
	void handleStore(llvm::StoreInst *store_inst);
	void handleGep(llvm::GetElementPtrInst *gep_inst);
	void handleCast(llvm::BitCastInst *cast_inst);
	void handleInst(llvm::Instruction *inst);
	void handleBlock(llvm::BasicBlock &blk);
	void handleFunc(llvm::Function &func);
	void handleMod(llvm::Module &mod);
	void handlePack();

public:
	void build();
};

#endif
