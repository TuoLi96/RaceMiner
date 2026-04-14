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
	fs::path db_source_info = db_root / "SourceInfo";
	fs::path db_compile = db_source_info / "compile.db";
	createDirIfNotExist(db_compile);
	path_json["db_compile"] = db_compile.string();
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
	fs::path tbl_select_file_lock_collection = tbl_race_root / "lock_collection.select_file";
	createDirIfNotExist(tbl_select_file_lock_collection);
	path_json["tbl_select_file_lock_collection"] = tbl_select_file_lock_collection.string();

	fs::path tbl_create_lock_use = tbl_race_root / "lock_use.create";
	createDirIfNotExist(tbl_create_lock_use);
	path_json["tbl_create_lock_use"] = tbl_create_lock_use.string();
	fs::path tbl_insert_lock_use = tbl_race_root / "lock_use.insert";
	createDirIfNotExist(tbl_insert_lock_use);
	path_json["tbl_insert_lock_use"] = tbl_insert_lock_use.string();
	fs::path tbl_select_lock_use = tbl_race_root / "lock_use.select";
	createDirIfNotExist(tbl_select_lock_use);
	path_json["tbl_select_lock_use"] = tbl_select_lock_use.string();
	fs::path tbl_select_file_lock_use = tbl_race_root / "lock_use.select_file";
	createDirIfNotExist(tbl_select_file_lock_use);
	path_json["tbl_select_file_lock_use"] = tbl_select_file_lock_use.string();



	/** DBSchema/SourceInfo */
	fs::path tbl_source_info_root = db_schema_root / "SourceInfo";
	/*** DBSchema/SourceInfo/Compile */
	fs::path tbl_compile_root = tbl_source_info_root / "Compile";
	fs::path tbl_create_link = tbl_compile_root / "link.create";
	createDirIfNotExist(tbl_create_link);
	path_json["tbl_create_link"] = tbl_create_link.string();
	fs::path tbl_insert_link = tbl_compile_root / "link.insert";
	createDirIfNotExist(tbl_insert_link);
	path_json["tbl_insert_link"] = tbl_insert_link.string();
	fs::path tbl_select_link = tbl_compile_root / "link.select";
	createDirIfNotExist(tbl_select_link);
	path_json["tbl_select_link"] = tbl_select_link.string();

}

void PathMgr::createConfigIfNotExist() {
	fs::path api_lock_path = work_root / "config" / "API" / "LockAPI";
	path_json["api_lock_path"] = api_lock_path.string();
	createDirIfNotExist(api_lock_path);
}

void PathMgr::createResultIfNotExist() {
	/* Results */
	fs::path result_path = work_root / "Results";

	/** Results/Race */
	fs::path result_race_path = result_path / "Race";
	/*** Results/Race/LockUse */
	fs::path result_race_lock_use_path = result_race_path / "LockUse.json";
	path_json["result_race_lock_use_path"] = result_race_lock_use_path.string();
	createDirIfNotExist(result_race_lock_use_path);
}

void PathMgr::createIfNotExist() {
	fs::path path_path = work_root / "config" / "Path" / "Path.json";
	if (fs::exists(path_path)) {
		return;
	}
	fs::create_directories(path_path.parent_path());
	
	createDBIfNotExist();
	createConfigIfNotExist();
	createResultIfNotExist();

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

string PathMgr::getTblSelectFileLockCollection() {
	return path_json["tbl_select_file_lock_collection"];
}

string PathMgr::getTblCreateLockUse() {
	return path_json["tbl_create_lock_use"];
}

string PathMgr::getTblInsertLockUse() {
	return path_json["tbl_insert_lock_use"];
}

string PathMgr::getTblSelectLockUse() {
	return path_json["tbl_select_lock_use"];
}

string PathMgr::getTblSelectFileLockUse() {
	return path_json["tbl_select_file_lock_use"];
}

string PathMgr::getCompileDBPath() {
	return path_json["db_compile"];
}

string PathMgr::getTblCreateLink() {
	return path_json["tbl_create_link"];
}

string PathMgr::getTblInsertLink() {
	return path_json["tbl_insert_link"];
}

string PathMgr::getTblSelectLink() {
	return path_json["tbl_select_link"];
}

string PathMgr::getResultRaceLockUsePath() {
	return path_json["result_race_lock_use_path"];
}
