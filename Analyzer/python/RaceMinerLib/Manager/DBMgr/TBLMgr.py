import logging
from pathlib import Path
from typing import Optional

from RaceMinerLib.Manager.DBMgr import DBMgr


class TBLMgr:
    def __init__(self, db_mgr):
        self.db_mgr = db_mgr

        # ========= logging =========
        self.logger = logging.getLogger(self.__class__.__name__)
        if not self.logger.handlers:
            handler = logging.StreamHandler()
            formatter = logging.Formatter(
                "[%(asctime)s][%(levelname)s] %(message)s"
            )
            handler.setFormatter(formatter)
            self.logger.addHandler(handler)
            self.logger.setLevel(logging.INFO)

    def load_sql(self, sql_path: str) -> Optional[str]:
        path = Path(sql_path)

        if not path.exists():
            self.logger.error(f"SQL file not found: {sql_path}")
            return None

        try:
            sql = path.read_text(encoding="utf-8")
            self.logger.debug(f"Loaded SQL file: {sql_path}")
            return sql
        except Exception as e:
            self.logger.error(f"Failed to read SQL file: {e}")
            return None
