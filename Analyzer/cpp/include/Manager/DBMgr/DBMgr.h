#ifndef _DB_MGR_H_
#define _DB_MGR_H_

#include <string>
#include <variant>
#include <vector>
#include <sqlite3.h>

#include "Manager/PathMgr/PathMgr.h"

using SqlValue = std::variant<
	std::nullptr_t,
	int,
	long long,
	std::string
>;

using SqlParams = std::vector<SqlValue>;

class DBMgr {
private:
	sqlite3 *db;
	std::vector<SqlParams> row_cache;
	size_t cache_size;

public:	
	DBMgr(std::string db_path, size_t cache_size = 0);
	~DBMgr();

private:
	std::string loadSql(std::string sql_path);
	bool bindValue(sqlite3_stmt *stmt, int index, const SqlValue &val);
	std::string params2str(const SqlParams &sql_params);
	bool beginTransaction();
	bool commit();
	bool rollback();

public:
	sqlite3 *getDB();
	bool execSimple(const std::string &sql);
	bool execSql(const std::string &sql, const SqlParams &params);
	bool execSqlBatch(const std::string &sql, const std::vector<SqlParams> &rows);

	sqlite3_stmt *prepareQuery(const std::string &sql, const SqlParams &params);
	void finalizeQuery(sqlite3_stmt *stmt);
};

#endif
