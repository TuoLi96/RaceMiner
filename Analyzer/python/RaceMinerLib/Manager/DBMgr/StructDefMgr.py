import logging
from dataclasses import dataclass
from typing import List

from RaceMinerLib.Manager.DBMgr import DBMgr
from RaceMinerLib.Manager.DBMgr import TBLMgr
from RaceMinerLib.Manager.PathMgr import PathMgr


@dataclass
class StructDefRow:
    stype_name: str
    typedef_type: str
    src_file: str
    start_line: int
    end_line: int
    start_byte: int
    end_byte: int


class StructDefMgr(TBLMgr):
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

        sql_path = self.path_mgr.get_tbl_create_struct_def()
        sql = self.load_sql(sql_path)

        if sql:
            self.db_mgr.exec_simple(sql)
            self.logger.info("StructDef table initialized")
        else:
            self.logger.error("Failed to initialize StructDef table")

    def insert(self, row: StructDefRow) -> bool:
        sql_path = self.path_mgr.get_tbl_insert_struct_def()
        sql = self.load_sql(sql_path)

        if not sql:
            return False

        params = [
            row.struct_name, row.is_static, row.is_inline,
            row.src_file, row.start_line, row.end_line,
            row.start_byte, row.end_byte
        ]

        return self.db_mgr.exec_sql(sql, params)

    def insert_batch(self, row_list: List[StructDefRow]) -> bool:
        if not row_list:
            return True

        sql_path = self.path_mgr.get_tbl_insert_struct_def()
        sql = self.load_sql(sql_path)

        if not sql:
            return False

        params_list = [
            [
                r.struct_name, r.is_static, r.is_inline,
                r.src_file, r.start_line, r.end_line,
                r.start_byte, r.end_byte
            ]
            for r in row_list
        ]

        return self.db_mgr.exec_sql_batch(sql, params_list)

    def select_all(self) -> List[StructDefRow]:
        sql_path = self.path_mgr.get_tbl_select_struct_def()
        sql = self.load_sql(sql_path)

        results: List[StructDefRow] = []

        if not sql:
            return results

        cur = self.db_mgr.prepare_query(sql, [])
        if not cur:
            return results

        try:
            rows = cur.fetchall()

            for r in rows:
                row = StructDefRow(
                    stype_name=r[1],
                    typedef_type=r[2],
                    src_file=r[3],
                    start_line=r[4],
                    end_line=r[5],
                    start_byte=r[6],
                    end_byte=r[7],
                )
                results.append(row)

        except Exception as e:
            self.logger.error(f"[select_all] {e}")

        finally:
            self.db_mgr.finalize_query(cur)

        return results

    def select_struct(self, struct) -> List[StructDefRow]:
        sql_path = self.path_mgr.get_tbl_select_struct_struct_def()
        sql = self.load_sql(sql_path)

        results: List[StructDefRow] = []

        if not sql:
            return results

        cur = self.db_mgr.prepare_query(sql, [struct])
        if not cur:
            return results

        try:
            rows = cur.fetchall()

            for r in rows:
                row = StructDefRow(
                    stype_name=r[1],
                    typedef_type = r[2],
                    src_file=r[3],
                    start_line=r[4],
                    end_line=r[5],
                    start_byte=r[6],
                    end_byte=r[7],
                )
                results.append(row)

        except Exception as e:
            self.logger.error(f"[select_all] {e}")

        finally:
            self.db_mgr.finalize_query(cur)

        return results
