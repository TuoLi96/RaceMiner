#ifndef _TBL_MGR_H_
#define _TBL_MGR_H_

#include "Manager/DBMgr/DBMgr.h"

class TBLMgr {
protected:
	DBMgr *db_mgr;

public:
	TBLMgr(DBMgr *db_mgr);
	~TBLMgr();

protected:
	std::string loadSql(std::string sql_path);
};

#endif
