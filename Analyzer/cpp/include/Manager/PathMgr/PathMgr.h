#ifndef _PATH_MGR_H_
#define _PATH_MGR_H_

#include <filesystem>

#include "3party/json/json.hpp"

class PathMgr {
private:
	std::filesystem::path miner_root; 
	std::filesystem::path work_root;
	nlohmann::json path_json;

public:
	PathMgr();
	~PathMgr();

private:
	void envError(std::string method, std::string env_var);

	void createDirIfNotExist(std::filesystem::path path);
	void createDBIfNotExist();
	void createAPIIfNotExist();
	void createIfNotExist();
	void load();

public:
	std::string getAPILockPath();

	std::string getRaceDBPath();
	std::string getTblCreateLockCollection();
	std::string getTblInsertLockCollection();
	std::string getTblSelectLockCollection();

	std::string getCompileDBPath();
	std::string getTblCreateLink();
	std::string getTblInsertLink();
	std::string getTblSelectLink();

};

#endif
