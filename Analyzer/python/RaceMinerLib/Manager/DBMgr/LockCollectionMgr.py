import logging
from dataclasses import dataclass
from typing import List

from RaceMinerLib.Manager.DBMgr import DBMgr
from RaceMinerLib.Manager.DBMgr import TBLMgr
from RaceMinerLib.Manager.PathMgr import PathMgr


@dataclass
class LockCollectionRow:
    file: str
    func: str
    lock_func: str
    lock_var: str
    lock_line: int
    unlock_func: str
    unlock_var: str
    unlock_line: int
    access_var: str
    access_line: int
    access_type: str


class LockCollectionMgr(TBLMgr):
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

        sql_path = self.path_mgr.get_tbl_create_lock_collection()
        sql = self.load_sql(sql_path)

        if sql:
            self.db_mgr.exec_simple(sql)
            self.logger.info("LockCollection table initialized")
        else:
            self.logger.error("Failed to initialize LockCollection table")

    def insert(self, row: LockCollectionRow) -> bool:
        sql_path = self.path_mgr.get_tbl_insert_lock_collection()
        sql = self.load_sql(sql_path)

        if not sql:
            return False

        params = [
            row.file, row.func,
            row.lock_func, row.lock_var, row.lock_line,
            row.unlock_func, row.unlock_var, row.unlock_line,
            row.access_var, row.access_line, row.access_type
        ]

        return self.db_mgr.exec_sql(sql, params)

    def insert_batch(self, row_list: List[LockCollectionRow]) -> bool:
        if not row_list:
            return True

        sql_path = self.path_mgr.get_tbl_insert_lock_collection()
        sql = self.load_sql(sql_path)

        if not sql:
            return False

        params_list = [
            [
                r.file, r.func,
                r.lock_func, r.lock_var, r.lock_line,
                r.unlock_func, r.unlock_var, r.unlock_line,
                r.access_var, r.access_line, r.access_type
            ]
            for r in row_list
        ]

        return self.db_mgr.exec_sql_batch(sql, params_list)

    def select_all(self) -> List[LockCollectionRow]:
        sql_path = self.path_mgr.get_tbl_select_lock_collection()
        sql = self.load_sql(sql_path)

        results: List[LockCollectionRow] = []

        if not sql:
            return results

        cur = self.db_mgr.prepare_query(sql, [])
        if not cur:
            return results

        try:
            rows = cur.fetchall()

            for r in rows:
                row = LockCollectionRow(
                    file=r[1],
                    func=r[2],
                    lock_func=r[3],
                    lock_var=r[4],
                    lock_line=r[5],
                    unlock_func=r[6],
                    unlock_var=r[7],
                    unlock_line=r[8],
                    access_var=r[9],
                    access_line=r[10],
                    access_type=r[11],
                )
                results.append(row)

        except Exception as e:
            self.logger.error(f"[select_all] {e}")

        finally:
            self.db_mgr.finalize_query(cur)

        return results
