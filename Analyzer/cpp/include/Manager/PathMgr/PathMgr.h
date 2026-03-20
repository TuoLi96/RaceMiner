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

	void createFileIfNotExist(std::filesystem::path path);
	void createAPIIfNotExist();
	void createIfNotExist();
	void load();

public:
	std::string getAPILockPath();
};

#endif
