#ifndef _LOCK_USE_MGR_H_
#define _LOCK_USE_MGR_H_

#include <string>
#include <vector>

#include "Manager/DBMgr/TBLMgr.h"
#include "Manager/PathMgr/PathMgr.h"

struct LockUseRow {
	std::string target_file;
	std::string lock_var;
	std::string access_var;
	int access_count;
};

class LockUseMgr : public TBLMgr {
private:
	PathMgr *path_mgr;
	DBMgr *db_mgr;

public:
	LockUseMgr(PathMgr *path_mgr, DBMgr *db_mgr);
	~LockUseMgr();

public:
	bool insert(LockUseRow &row);
	bool insertBatch(std::vector<LockUseRow> &row_vec);
	std::vector<LockUseRow> selectAll();
	std::vector<LockUseRow> selectTarget(std::string target_file);
};

#endif
