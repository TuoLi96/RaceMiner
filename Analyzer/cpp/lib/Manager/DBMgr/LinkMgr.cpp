#include "Manager/DBMgr/LinkMgr.h"

#include "Utils/DBOperations.h"

using namespace std;

/*
 * Implementation of LinkMgr
 */

LinkMgr::LinkMgr(PathMgr *path_mgr, DBMgr *db_mgr) : TBLMgr(db_mgr) {
	this->path_mgr = path_mgr;
	this->db_mgr = db_mgr;
	string tbl_create_link = path_mgr->getTblCreateLink();
	string tbl_create_link_sql = loadSql(tbl_create_link);
	db_mgr->execSimple(tbl_create_link_sql);
}

LinkMgr::~LinkMgr() {
	// Need to do nothing at present.
}

bool LinkMgr::insert(LinkRow &row) {
	SqlParams sql_params = {row.target_file, row.link_list, row.ir_list}; 
	string tbl_insert_link = path_mgr->getTblInsertLink();
	string tbl_insert_link_sql = loadSql(tbl_insert_link);
	return db_mgr->execSql(tbl_insert_link_sql, sql_params);
}

bool LinkMgr::insertBatch(vector<LinkRow> &row_vec) {
	vector<SqlParams> params_list;
	params_list.reserve(row_vec.size());

	for (size_t row_idx = 0; row_idx < row_vec.size(); row_idx++) {
		LinkRow &row = row_vec[row_idx];
		params_list.push_back({row.target_file, row.link_list, row.ir_list});
	}
	string tbl_insert_link = path_mgr->getTblInsertLink();
	string tbl_insert_link_sql = loadSql(tbl_insert_link);
	return db_mgr->execSqlBatch(tbl_insert_link_sql, params_list);
}

vector<LinkRow> LinkMgr::selectAll() {
	string tbl_select_link = path_mgr->getTblSelectLink();
	string tbl_select_link_sql = loadSql(tbl_select_link);
	vector<LinkRow> results;

	sqlite3_stmt * stmt = db_mgr->prepareQuery(tbl_select_link_sql, {});
	if (!stmt) {
		return results;
	}
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		LinkRow row;
		row.target_file = get_column_text(stmt, 0);
		row.link_list = get_column_text(stmt, 1);
		row.ir_list = get_column_text(stmt, 2);

		results.push_back(row);
	}

	db_mgr->finalizeQuery(stmt);
	return results;
}
