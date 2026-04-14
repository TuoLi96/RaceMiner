#include "Manager/DBMgr/LockUseMgr.h"

#include "Utils/DBOperations.h"

using namespace std;

/*
 * Implementation of LockUseMgr
 */

LockUseMgr::LockUseMgr(PathMgr *path_mgr, DBMgr *db_mgr) : TBLMgr(db_mgr) {
	this->path_mgr = path_mgr;
	this->db_mgr = db_mgr;
	string tbl_create_lock_use = path_mgr->getTblCreateLockUse();
	string tbl_create_lock_use_sql = loadSql(tbl_create_lock_use);
	db_mgr->execSimple(tbl_create_lock_use_sql);
}

LockUseMgr::~LockUseMgr() {
	// Need to do nothing at present.
}

bool LockUseMgr::insert(LockUseRow &row) {
	SqlParams sql_params = {row.target_file, row.lock_var, row.access_var, row.access_count};
	string tbl_insert_lock_use = path_mgr->getTblInsertLockUse();
	string tbl_insert_lock_use_sql = loadSql(tbl_insert_lock_use);
	return db_mgr->execSql(tbl_insert_lock_use_sql, sql_params);
}

bool LockUseMgr::insertBatch(vector<LockUseRow> &row_vec) {
	vector<SqlParams> params_list;
	params_list.reserve(row_vec.size());

	for (size_t row_idx = 0; row_idx < row_vec.size(); row_idx++) {
		LockUseRow &row = row_vec[row_idx];
		params_list.push_back({row.target_file, row.lock_var, row.access_var, row.access_count});
	}
	string tbl_insert_lock_use = path_mgr->getTblInsertLockUse();
	string tbl_insert_lock_use_sql = loadSql(tbl_insert_lock_use);
	return db_mgr->execSqlBatch(tbl_insert_lock_use_sql, params_list);
}

vector<LockUseRow> LockUseMgr::selectAll() {
	string tbl_select_lock_use = path_mgr->getTblSelectLockUse();
	string tbl_select_lock_use_sql = loadSql(tbl_select_lock_use);
	vector<LockUseRow> results;

	sqlite3_stmt * stmt = db_mgr->prepareQuery(tbl_select_lock_use_sql, {});
	if (!stmt) {
		return results;
	}
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		LockUseRow row;
		row.target_file = get_column_text(stmt, 1);
		row.lock_var = get_column_text(stmt, 2);
		row.access_var = get_column_text(stmt, 3);;
		row.access_count = get_column_int(stmt, 4);
		results.push_back(row);
	}

	db_mgr->finalizeQuery(stmt);
	return results;
}

vector<LockUseRow> LockUseMgr::selectTarget(string target_file) {
	string tbl_select_target_lock_use = path_mgr->getTblSelectFileLockUse();
	string tbl_select_target_lock_use_sql = loadSql(tbl_select_target_lock_use);
	vector<LockUseRow> results;

	sqlite3_stmt * stmt = db_mgr->prepareQuery(tbl_select_target_lock_use_sql, {target_file});
	if (!stmt) {
		return results;
	}
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		LockUseRow row;
		row.target_file = get_column_text(stmt, 1);
		row.lock_var = get_column_text(stmt, 2);
		row.access_var = get_column_text(stmt, 3);;
		row.access_count = get_column_int(stmt, 4);
		results.push_back(row);
	}

	db_mgr->finalizeQuery(stmt);
	return results;
}
