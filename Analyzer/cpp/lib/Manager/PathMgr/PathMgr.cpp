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

void PathMgr::createDirIfNotExist(fs::path path) {
	fs::create_directories(path.parent_path());
	/*if (!fs::exists(path)) {
		ofstream ofs(path.string());
		ofs.close();
	}*/
}

void PathMgr::createDBIfNotExist() {
	/* Database */
	fs::path db_root = work_root / "Database";
	/** Database/SourceInfo */

	/** DataBase/BugInfo */
	fs::path db_bug = db_root / "BugInfo";
	fs::path db_bug_race = db_bug / "Race.db";
	createDirIfNotExist(db_bug_race);
	path_json["db_bug_race"] = db_bug_race.string();
	/* DBSchema */
	fs::path db_schema_root = miner_root / "DBSchema";
	/** DBSchema/BugDetection */
	fs::path tbl_bug_detection_root = db_schema_root / "BugDetection";
	/*** DBSchema/BugDetection/Race */
	fs::path tbl_race_root = tbl_bug_detection_root / "Race";
	fs::path tbl_create_lock_collection = tbl_race_root / "lock_collection.create";
	createDirIfNotExist(tbl_create_lock_collection);
	path_json["tbl_create_lock_collection"] = tbl_create_lock_collection.string();
	fs::path tbl_insert_lock_collection = tbl_race_root / "lock_collection.insert";
	createDirIfNotExist(tbl_insert_lock_collection);
	path_json["tbl_insert_lock_collection"] = tbl_insert_lock_collection.string();
	fs::path tbl_select_lock_collection = tbl_race_root / "lock_collection.select";
	createDirIfNotExist(tbl_select_lock_collection);
	path_json["tbl_select_lock_collection"] = tbl_select_lock_collection.string();
}

void PathMgr::createAPIIfNotExist() {
	fs::path api_lock_path = work_root / "config" / "API" / "LockAPI";
	path_json["api_lock_path"] = api_lock_path.string();
	createDirIfNotExist(api_lock_path);
}

void PathMgr::createIfNotExist() {
	fs::path path_path = work_root / "config" / "Path" / "Path.json";
	if (fs::exists(path_path)) {
		return;
	}
	fs::create_directories(path_path.parent_path());
	
	createAPIIfNotExist();
	createDBIfNotExist();

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

string PathMgr::getRaceDBPath() {
	return path_json["db_bug_race"];
}

string PathMgr::getTblCreateLockCollection() {
	return path_json["tbl_create_lock_collection"];
}

string PathMgr::getTblInsertLockCollection() {
	return path_json["tbl_insert_lock_collection"];
}

string PathMgr::getTblSelectLockCollection() {
	return path_json["tbl_select_lock_collection"];
}
