import logging
from dataclasses import dataclass
from typing import List

from RaceMinerLib.Manager.DBMgr import DBMgr
from RaceMinerLib.Manager.DBMgr import TBLMgr
from RaceMinerLib.Manager.PathMgr import PathMgr


@dataclass
class IncludeInfoRow:
    main_file: str
    include_files: str


class IncludeInfoMgr(TBLMgr):
    def __init__(self, path_mgr, db_mgr):
        super().__init__(db_mgr)

        self.path_mgr = path_mgr
        self.db_mgr = db_mgr

        self.logger = logging.getLogger(self.__class__.__name__)
        if not self.logger.handlers:
            handler = logging.StreamHandler()
            formatter = logging.Formatter(
                "[%(asctime)s][%(levelname)s] %(message)s"
            )
            handler.setFormatter(formatter)
            self.logger.addHandler(handler)
            self.logger.setLevel(logging.INFO)

        sql_path = self.path_mgr.get_tbl_create_include_info()
        sql = self.load_sql(sql_path)

        if sql:
            self.db_mgr.exec_simple(sql)
            self.logger.info("IncludeInfo table initialized")
        else:
            self.logger.error("Failed to initialize IncludeInfo table")

    def insert(self, row: IncludeInfoRow) -> bool:
        sql_path = self.path_mgr.get_tbl_insert_include_info()
        sql = self.load_sql(sql_path)

        if not sql:
            return False

        params = [
            row.main_file, row.include_files
        ]

        return self.db_mgr.exec_sql(sql, params)

    def insert_batch(self, row_list: List[IncludeInfoRow]) -> bool:
        if not row_list:
            return True

        sql_path = self.path_mgr.get_tbl_insert_include_info()
        sql = self.load_sql(sql_path)

        if not sql:
            return False

        params_list = [
            [
                r.main_file, r.include_files
            ]
            for r in row_list
        ]

        return self.db_mgr.exec_sql_batch(sql, params_list)

    def select_all(self) -> List[IncludeInfoRow]:
        sql_path = self.path_mgr.get_tbl_select_include_info()
        sql = self.load_sql(sql_path)

        results: List[IncludeInfoRow] = []

        if not sql:
            return results

        cur = self.db_mgr.prepare_query(sql, [])
        if not cur:
            return results

        try:
            rows = cur.fetchall()

            for r in rows:
                row = IncludeInfoRow(
                    main_file=r[1],
                    include_files=r[2],
                )
                results.append(row)

        except Exception as e:
            self.logger.error(f"[select_all] {e}")

        finally:
            self.db_mgr.finalize_query(cur)

        return results

    def select_file(self, file) -> List[IncludeInfoRow]:
        sql_path = self.path_mgr.get_tbl_select_file_include_info()
        sql = self.load_sql(sql_path)

        results: List[IncludeInfoRow] = []

        if not sql:
            return results

        cur = self.db_mgr.prepare_query(sql, [file])
        if not cur:
            return results

        try:
            rows = cur.fetchall()

            for r in rows:
                row = IncludeInfoRow(
                    main_file=r[1],
                    include_files=r[2],
                )
                results.append(row)

        except Exception as e:
            self.logger.error(f"[select_all] {e}")

        finally:
            self.db_mgr.finalize_query(cur)

        return results
