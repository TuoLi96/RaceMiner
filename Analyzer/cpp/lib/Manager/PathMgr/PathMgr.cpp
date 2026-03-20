#include "Manager/PathMgr/PathMgr.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>

#include <spdlog/spdlog.h>

using namespace std;
namespace fs = std::filesystem;

/*
 * Implementation of PathMgr
 */

PathMgr::PathMgr() {
	const char* miner_root_str = getenv("MINER_ROOT");
	if (miner_root_str == NULL) {
		envError("PathMgr", "MINER_ROOT");
		miner_root = "";
	} else {
		miner_root = string(miner_root_str);
	}
	const char* work_root_str = getenv("MINER_WORK_ROOT");
	if (work_root_str == NULL) {
		envError("PathMgr", "MINER_WORK_ROOT");
		work_root = "";
	} else {
		work_root = string(work_root_str);
	}
	createIfNotExist();
	load();
}

PathMgr::~PathMgr() {
	// Need to do nothing at present.
}

void PathMgr::envError(string method, string env_var) {
	spdlog::error("PathMgr::{}: Environment not set: {}", method, env_var);
}

void PathMgr::createFileIfNotExist(fs::path path) {
	fs::create_directories(path.parent_path());
	if (!fs::exists(path)) {
		ofstream ofs(path.string());
		ofs.close();
	}
}

void PathMgr::createAPIIfNotExist() {
	fs::path api_lock_path = work_root / "config" / "API" / "LockAPI";
	path_json["api_lock_path"] = api_lock_path.string();
	createFileIfNotExist(api_lock_path);
}

void PathMgr::createIfNotExist() {
	fs::path path_path = work_root / "config" / "Path" / "Path.json";
	if (fs::exists(path_path)) {
		return;
	}
	fs::create_directories(path_path.parent_path());
	
	createAPIIfNotExist();

	ofstream json_file(path_path.string());
	json_file << path_json.dump(4);
}

void PathMgr::load() {
	fs::path path_path = work_root / "config" / "Path" / "Path.json";
	ifstream path_file(path_path.string());
	path_file >> path_json;
}

string PathMgr::getAPILockPath() {
	return path_json["api_lock_path"];
}
