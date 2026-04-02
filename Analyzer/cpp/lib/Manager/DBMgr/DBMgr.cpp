#include "Manager/DBMgr/DBMgr.h"

#include <string>
#include <fstream>
#include <iostream>
#include <spdlog/spdlog.h>

using namespace std;

/*
 * Implementation of DBMgr
 */

DBMgr::DBMgr(string db_path, size_t cache_size) {
	db = NULL;
	if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
		spdlog::error("DBMgr::DBMgr: Fail to open database: {}", db_path);
	}
	execSimple("PRAGMA journal_mode=WAL;");
	execSimple("PRAGMA foreign_keys=ON;");
	row_cache.clear();
	this->cache_size = cache_size;
}

DBMgr::~DBMgr() {
	sqlite3_close(db);
}

string DBMgr::loadSql(string sql_path) {
	ifstream ifs(sql_path);
	stringstream buffer;
	buffer << ifs.rdbuf();
	return buffer.str();
}

bool DBMgr::execSimple(const string &sql) {
	char *err_msg = nullptr;
	int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg);
	if (rc != SQLITE_OK) {
		sqlite3_free(err_msg);
		return false;
	}
	return true;
}

bool DBMgr::bindValue(sqlite3_stmt *stmt, int index, const SqlValue &val) {
	return visit([&](auto &&arg) -> bool {
		using T = decay_t<decltype(arg)>;
		if constexpr (is_same_v<T, nullptr_t>) {
			return sqlite3_bind_null(stmt, index) == SQLITE_OK;
		} else if constexpr (is_same_v<T, int>) {
			return sqlite3_bind_int(stmt, index, arg) == SQLITE_OK;
		} else if constexpr (is_same_v<T, long long>) {
			return sqlite3_bind_int64(stmt, index, arg) == SQLITE_OK;
		} else if constexpr (is_same_v<T, string>) {
			return sqlite3_bind_text(stmt, index, arg.c_str(), -1, SQLITE_TRANSIENT) == SQLITE_OK;
		} else {
			return false;
		}
	}, val);
}

std::string DBMgr::params2str(const SqlParams &sql_params) {
	string result;
	for (auto &param : sql_params) {
		string param_str = std::visit([](const auto &x) -> string {
			using T = decay_t<decltype(x)>;
			if constexpr (is_same_v<T, nullptr_t>) {
				return "NULL";
			} else if constexpr (is_same_v<T, string>) {
				return x;
			} else {
				return to_string(x);
			}
		}, param);
		result = result + param_str + " ";
	}
	return result;
}

bool DBMgr::beginTransaction() {
	return execSimple("BEGIN TRANSACTION;");
}

bool DBMgr::commit() {
	return execSimple("COMMIT;");
}

bool DBMgr::rollback() {
	return execSimple("ROLLBACK;");
}

bool DBMgr::execSql(const std::string &sql, const SqlParams &params) {
	if (cache_size) {
		if (row_cache.size() < cache_size) {
			row_cache.push_back(params);
			// FIXME: Return true without other checking may be wrong.
			return true;
		} else {
			bool ret = execSqlBatch(sql, row_cache);
			// FIXME: Clear whatever ret is true.
			row_cache.clear();
			return ret;
		}
	} 
	sqlite3_stmt *stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
		spdlog::error("DBMgr::execSql: Prepare failed.");
		return false;
	}
	for (size_t param_idx = 0; param_idx < params.size(); param_idx++) {
		if (!bindValue(stmt, param_idx + 1, params[param_idx])) {
			spdlog::error("DBMgr::execSql: Bind failed at param {}", param_idx + 1);
			sqlite3_finalize(stmt);
			return false;
		}
	}
	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		spdlog::error("DBMgr::execSql: Execution failed.");
		sqlite3_finalize(stmt);
		return false;
	} else {
		int changes = sqlite3_changes(db);
		if (changes == 0) {
			//spdlog::warn("DBMgr::execSql: Repeated insert:\n {}", params2str(params));
		}
	}
	sqlite3_finalize(stmt);
	return true;
}

bool DBMgr::execSqlBatch(const std::string &sql, const std::vector<SqlParams> &rows) {
	if (rows.empty()) {
		return true;
	}

	sqlite3_stmt *stmt = nullptr;

	if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) !=SQLITE_OK) {
		spdlog::error("DBMgr::execSqlBatch: Prepare failed.");
		return false;
	}

	if (!beginTransaction()) {
		sqlite3_finalize(stmt);
		return false;
	}

	bool success = true;
	for (const auto &row : rows) {
		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);

		for (size_t param_idx = 0; param_idx < row.size(); param_idx++) {
			if (!bindValue(stmt, param_idx + 1, row[param_idx])) {
				spdlog::error("DBMgr::execSqlBatch: Bind failed at param {}", param_idx + 1);
				success = false;
				break;
			}
		}
		if (!success) {
			break;
		}

		int rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE) {
			spdlog::error("DBMgr::execSqlBatch: Execution failed.");
			success = false;
			break;
		}
	}
	if (success) {
		if (!commit()) {
			success = false;
		}
	} else {
		rollback();
	}
	sqlite3_finalize(stmt);
	return success;
}

sqlite3_stmt *DBMgr::prepareQuery(const string &sql, const SqlParams &params) {
	sqlite3_stmt *stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
		spdlog::error("DBMgr::prepareQuery: Prepare query failed.");
		return nullptr;
	}
	for (size_t param_idx = 0; param_idx < params.size(); param_idx++) {
		if (!bindValue(stmt, param_idx + 1, params[param_idx])) {
			spdlog::error("DBMgr::prepareQuery: Bind query param failed at {}", param_idx + 1);
			sqlite3_finalize(stmt);
			return nullptr;
		}
	}
	return stmt;
}

void DBMgr::finalizeQuery(sqlite3_stmt *stmt) {
	if (stmt) {
		sqlite3_finalize(stmt);
	}
}
