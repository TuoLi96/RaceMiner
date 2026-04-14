#include "Manager/DBMgr/LockCollectionMgr.h"

#include "Utils/DBOperations.h"

using namespace std;

/*
 * Implementation of LockCollectionMgr
 */

LockCollectionMgr::LockCollectionMgr(PathMgr *path_mgr, DBMgr *db_mgr) : TBLMgr(db_mgr) {
	this->path_mgr = path_mgr;
	this->db_mgr = db_mgr;
	string tbl_create_lock_collection = path_mgr->getTblCreateLockCollection();
	string tbl_create_lock_collection_sql = loadSql(tbl_create_lock_collection);
	db_mgr->execSimple(tbl_create_lock_collection_sql);
}

LockCollectionMgr::~LockCollectionMgr() {
	// Need to do nothing at present.
}

bool LockCollectionMgr::insert(LockCollectionRow &row) {
	SqlParams sql_params = {row.file, row.func, 
					row.lock_func, row.lock_var, row.lock_line,
					row.unlock_func, row.unlock_var, row.unlock_line,
					row.access_var, row.access_line, row.access_type};
	string tbl_insert_lock_collection = path_mgr->getTblInsertLockCollection();
	string tbl_insert_lock_collection_sql = loadSql(tbl_insert_lock_collection);
	return db_mgr->execSql(tbl_insert_lock_collection_sql, sql_params);
}

bool LockCollectionMgr::insertBatch(vector<LockCollectionRow> &row_vec) {
	vector<SqlParams> params_list;
	params_list.reserve(row_vec.size());

	for (size_t row_idx = 0; row_idx < row_vec.size(); row_idx++) {
		LockCollectionRow &row = row_vec[row_idx];
		params_list.push_back({row.file, row.func,
					row.lock_func, row.lock_var, row.lock_line,
					row.unlock_func, row.unlock_var, row.unlock_line,
					row.access_var, row.access_line, row.access_type});
	}
	string tbl_insert_lock_collection = path_mgr->getTblInsertLockCollection();
	string tbl_insert_lock_collection_sql = loadSql(tbl_insert_lock_collection);
	return db_mgr->execSqlBatch(tbl_insert_lock_collection_sql, params_list);
}

vector<LockCollectionRow> LockCollectionMgr::selectAll() {
	string tbl_select_lock_collection = path_mgr->getTblSelectLockCollection();
	string tbl_select_lock_collection_sql = loadSql(tbl_select_lock_collection);
	vector<LockCollectionRow> results;

	sqlite3_stmt * stmt = db_mgr->prepareQuery(tbl_select_lock_collection_sql, {});
	if (!stmt) {
		return results;
	}
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		LockCollectionRow row;
		row.file = get_column_text(stmt, 1);
		row.func = get_column_text(stmt, 2);
		row.lock_func = get_column_text(stmt, 3);
		row.lock_var = get_column_text(stmt, 4);
		row.lock_line = get_column_int(stmt, 5);
		row.unlock_func = get_column_text(stmt, 6);
		row.unlock_var = get_column_text(stmt, 7);
		row.unlock_line = get_column_int(stmt, 8);
		row.access_var = get_column_text(stmt, 9);;
		row.access_line = get_column_int(stmt, 10);
		row.access_type = get_column_text(stmt, 11);

		results.push_back(row);
	}

	db_mgr->finalizeQuery(stmt);
	return results;
}

vector<LockCollectionRow> LockCollectionMgr::selectFile(string src_file) {
	string tbl_select_file_lock_collection = path_mgr->getTblSelectFileLockCollection();
	string tbl_select_file_lock_collection_sql = loadSql(tbl_select_file_lock_collection);
	vector<LockCollectionRow> results;

	sqlite3_stmt * stmt = db_mgr->prepareQuery(tbl_select_file_lock_collection_sql, {src_file});
	if (!stmt) {
		return results;
	}
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		LockCollectionRow row;
		row.file = get_column_text(stmt, 1);
		row.func = get_column_text(stmt, 2);
		row.lock_func = get_column_text(stmt, 3);
		row.lock_var = get_column_text(stmt, 4);
		row.lock_line = get_column_int(stmt, 5);
		row.unlock_func = get_column_text(stmt, 6);
		row.unlock_var = get_column_text(stmt, 7);
		row.unlock_line = get_column_int(stmt, 8);
		row.access_var = get_column_text(stmt, 9);;
		row.access_line = get_column_int(stmt, 10);
		row.access_type = get_column_text(stmt, 11);

		results.push_back(row);
	}

	db_mgr->finalizeQuery(stmt);
	return results;
}
