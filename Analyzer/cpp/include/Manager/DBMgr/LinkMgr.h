#ifndef _LINK_MGR_H_
#define _LINK_MGR_H_

#include <string>
#include <vector>

#include "Manager/DBMgr/TBLMgr.h"
#include "Manager/PathMgr/PathMgr.h"

struct LinkRow {
	std::string target_file;
	std::string link_list;
	std::string ir_list;
};

class LinkMgr : public TBLMgr {
private:
	PathMgr *path_mgr;
	DBMgr *db_mgr;

public:
	LinkMgr(PathMgr *path_mgr, DBMgr *db_mgr);
	~LinkMgr();

public:
	bool insert(LinkRow &row);
	bool insertBatch(std::vector<LinkRow> &row_vec);
	std::vector<LinkRow> selectAll();
};

#endif
