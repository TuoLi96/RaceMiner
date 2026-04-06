import sqlite3
import logging
from typing import List, Any, Optional

SqlValue = Any
SqlParams = List[SqlValue]

class DBMgr:
    def __init__(self, db_path: str, cache_size: int = 0):
        self.logger = logging.getLogger("DBMgr")
        if not self.logger.handlers:
            handler = logging.StreamHandler()
            formatter = logging.Formatter(
                "[%(asctime)s][%(levelname)s] %(message)s"
            )
            handler.setFormatter(formatter)
            self.logger.addHandler(handler)
            self.logger.setLevel(logging.INFO)

        try:
            self.conn = sqlite3.connect(db_path)
            self.conn.execute("PRAGMA journal_mode=WAL;")
            self.conn.execute("PRAGMA foreign_keys=ON;")
            self.logger.info(f"Open database: {db_path}")
        except Exception as e:
            self.logger.error(f"Failed to open database: {e}")
            raise

        self.cache_size = cache_size
        self.row_cache: List[SqlParams] = []

    def close(self):
        try:
            self.conn.close()
            self.logger.info("Database closed")
        except Exception as e:
            self.logger.error(f"Close failed: {e}")

    def exec_simple(self, sql: str) -> bool:
        try:
            self.conn.execute(sql)
            self.conn.commit()
            self.logger.debug(f"Exec simple SQL: {sql}")
            return True
        except Exception as e:
            self.logger.error(f"[exec_simple] {e} | SQL: {sql}")
            return False

    def exec_sql(self, sql: str, params: SqlParams) -> bool:
        if self.cache_size > 0:
            self.row_cache.append(params)

            if len(self.row_cache) < self.cache_size:
                self.logger.debug(f"Cache row: {params}")
                return True
            else:
                self.logger.info("Cache full, flushing batch...")
                ret = self.exec_sql_batch(sql, self.row_cache)
                self.row_cache.clear()
                return ret

        try:
            cur = self.conn.execute(sql, params)
            self.conn.commit()

            if cur.rowcount == 0:
                self.logger.debug(f"No change: {params}")

            return True
        except Exception as e:
            self.logger.error(f"[exec_sql] {e} | SQL: {sql} | Params: {params}")
            return False

    def exec_sql_batch(self, sql: str, rows: List[SqlParams]) -> bool:
        if not rows:
            return True

        try:
            cur = self.conn.cursor()
            self.conn.execute("BEGIN TRANSACTION;")

            for idx, row in enumerate(rows):
                try:
                    cur.execute(sql, row)
                except Exception as e:
                    self.logger.error(f"[batch row {idx}] {e} | Params: {row}")
                    raise

            self.conn.commit()
            self.logger.info(f"Batch executed: {len(rows)} rows")
            return True

        except Exception as e:
            self.logger.error(f"[exec_sql_batch] rollback due to error: {e}")
            self.conn.rollback()
            return False

    def prepare_query(self, sql: str, params: SqlParams):
        try:
            cur = self.conn.cursor()
            cur.execute(sql, params)
            self.logger.debug(f"Query prepared: {sql} | {params}")
            return cur
        except Exception as e:
            self.logger.error(f"[prepare_query] {e} | SQL: {sql} | Params: {params}")
            return None

    def finalize_query(self, cursor):
        if cursor:
            cursor.close()

    def begin_transaction(self):
        try:
            self.conn.execute("BEGIN TRANSACTION;")
            self.logger.debug("Begin transaction")
        except Exception as e:
            self.logger.error(f"Begin transaction failed: {e}")

    def commit(self):
        try:
            self.conn.commit()
            self.logger.debug("Commit")
        except Exception as e:
            self.logger.error(f"Commit failed: {e}")

    def rollback(self):
        try:
            self.conn.rollback()
            self.logger.debug("Rollback")
        except Exception as e:
            self.logger.error(f"Rollback failed: {e}")

    def flush(self, sql: str):
        if self.row_cache:
            self.logger.info(f"Flushing remaining {len(self.row_cache)} rows...")
            self.exec_sql_batch(sql, self.row_cache)
            self.row_cache.clear()
