#include <memory>
#include <iostream>
#include <string>

#include "Manager/ModMgr/ModPack.h"
#include "AliasAnalysis/TypeGraph.h"

using namespace std;
using namespace llvm;

int main(int argc, char **argv) {
	ModPack *mod_pack = new ModPack();
	mod_pack->push(string(argv[1]));
	TypeGraph *type_graph = new TypeGraph(mod_pack);
	type_graph->analyze();
	type_graph->dumpSvg("alias_graph.svg");
	delete type_graph;
	delete mod_pack;
    return 0;
}
