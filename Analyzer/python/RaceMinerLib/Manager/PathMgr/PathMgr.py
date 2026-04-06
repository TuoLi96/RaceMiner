import os
import json
import logging
from pathlib import Path

class PathMgr:
    def __init__(self):
        self.logger = logging.getLogger("PathMgr")
        if not self.logger.handlers:
            handler = logging.StreamHandler()
            formatter = logging.Formatter(
                "[%(asctime)s][%(levelname)s] %(message)s"
            )
            handler.setFormatter(formatter)
            self.logger.addHandler(handler)
            self.logger.setLevel(logging.INFO)

        self.miner_root = self._get_env("MINER_ROOT")
        self.work_root = self._get_env("MINER_WORK_ROOT")

        self.path_json = {}

        self._create_if_not_exist()
        self._load()

    def _get_env(self, name: str) -> Path:
        val = os.getenv(name)
        if not val:
            self.logger.error(f"Environment not set: {name}")
            return Path("")
        return Path(val)

    def _create_dir_if_not_exist(self, path: Path):
        path.parent.mkdir(parents=True, exist_ok=True)

    def _create_db_if_not_exist(self):
        db_root = self.work_root / "Database"

        db_source_info = db_root / "SourceInfo"
        db_compile = db_source_info / "compile.db"
        self._create_dir_if_not_exist(db_compile)
        self.path_json["db_compile"] = str(db_compile)

        db_bug = db_root / "BugInfo"
        db_bug_race = db_bug / "Race.db"
        self._create_dir_if_not_exist(db_bug_race)
        self.path_json["db_bug_race"] = str(db_bug_race)

        db_schema_root = self.miner_root / "DBSchema"

        tbl_race_root = db_schema_root / "BugDetection" / "Race"

        def add_sql(name, path):
            self._create_dir_if_not_exist(path)
            self.path_json[name] = str(path)

        add_sql("tbl_create_lock_collection", tbl_race_root / "lock_collection.create")
        add_sql("tbl_insert_lock_collection", tbl_race_root / "lock_collection.insert")
        add_sql("tbl_select_lock_collection", tbl_race_root / "lock_collection.select")

        tbl_compile_root = db_schema_root / "SourceInfo" / "Compile"

        add_sql("tbl_create_link", tbl_compile_root / "link.create")
        add_sql("tbl_insert_link", tbl_compile_root / "link.insert")
        add_sql("tbl_select_link", tbl_compile_root / "link.select")

    def _create_api_if_not_exist(self):
        api_lock_path = self.work_root / "config" / "API" / "LockAPI"
        self._create_dir_if_not_exist(api_lock_path)
        self.path_json["api_lock_path"] = str(api_lock_path)

    def _create_if_not_exist(self):
        path_file = self.work_root / "config" / "Path" / "Path.json"

        if path_file.exists():
            return

        path_file.parent.mkdir(parents=True, exist_ok=True)

        self._create_api_if_not_exist()
        self._create_db_if_not_exist()

        with open(path_file, "w", encoding="utf-8") as f:
            json.dump(self.path_json, f, indent=4)

        self.logger.info(f"Path config created: {path_file}")

    def _load(self):
        path_file = self.work_root / "config" / "Path" / "Path.json"

        try:
            with open(path_file, "r", encoding="utf-8") as f:
                self.path_json = json.load(f)
        except Exception as e:
            self.logger.error(f"Failed to load Path.json: {e}")

    def get_api_lock_path(self):
        return self.path_json.get("api_lock_path")

    def get_race_db_path(self):
        return self.path_json.get("db_bug_race")

    def get_tbl_create_lock_collection(self):
        return self.path_json.get("tbl_create_lock_collection")

    def get_tbl_insert_lock_collection(self):
        return self.path_json.get("tbl_insert_lock_collection")

    def get_tbl_select_lock_collection(self):
        return self.path_json.get("tbl_select_lock_collection")

    def get_compile_db_path(self):
        return self.path_json.get("db_compile")

    def get_tbl_create_link(self):
        return self.path_json.get("tbl_create_link")

    def get_tbl_insert_link(self):
        return self.path_json.get("tbl_insert_link")

    def get_tbl_select_link(self):
        return self.path_json.get("tbl_select_link")
