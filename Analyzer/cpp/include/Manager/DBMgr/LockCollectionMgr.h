#ifndef _LOCK_COLLECTION_MGR_H_
#define _LOCK_COLLECTION_MGR_H_

#include <string>
#include <vector>

#include "Manager/DBMgr/TBLMgr.h"
#include "Manager/PathMgr/PathMgr.h"

struct LockCollectionRow {
	std::string file;
	std::string func;
	std::string lock_func;
	std::string lock_var;
	int lock_line;
	std::string unlock_func;
	std::string unlock_var;
	int unlock_line;
	std::string access_var;
	int access_line;
	std::string access_type;
};

class LockCollectionMgr : public TBLMgr {
private:
	PathMgr *path_mgr;
	DBMgr *db_mgr;

public:
	LockCollectionMgr(PathMgr *path_mgr, DBMgr *db_mgr);
	~LockCollectionMgr();

public:
	bool insert(LockCollectionRow &row);
	bool insertBatch(std::vector<LockCollectionRow> &row_vec);
	std::vector<LockCollectionRow> selectAll();
	std::vector<LockCollectionRow> selectFile(std::string src_file);
};

#endif
