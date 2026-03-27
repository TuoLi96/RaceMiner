#include "Manager/DBMgr/TBLMgr.h"

#include <fstream>

using namespace std;

/*
 * Implementation of TBLMgr
 */

TBLMgr::TBLMgr(DBMgr *db_mgr) {
	this->db_mgr = db_mgr;
}

TBLMgr::~TBLMgr() {
	// Need to do nothing at present.
}

string TBLMgr::loadSql(string sql_path) {
	ifstream ifs(sql_path);
	stringstream buffer;
	buffer << ifs.rdbuf();
	return buffer.str();
}

