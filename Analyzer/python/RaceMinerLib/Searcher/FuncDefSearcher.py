from pathlib import Path
from RaceMinerLib.Manager.PathMgr import PathMgr
from RaceMinerLib.Manager.DBMgr import *

class FuncDefSearcher:
    def __init__(self, path_mgr, srcinfo_db_mgr):
        self.path_mgr = path_mgr
        self.func_def_mgr = FuncDefMgr(path_mgr, srcinfo_db_mgr)
        self.include_info_mgr = IncludeInfoMgr(path_mgr, srcinfo_db_mgr)

    def get_func_def_text(self, func_def):
        file_path = func_def.src_file
        start_byte = func_def.start_byte
        end_byte = func_def.end_byte

        with open(file_path, "r", encoding="utf-8") as f:
            checked_src_root = self.path_mgr.get_checked_src_path() + "/"
            source_code = f.read()
            func_def_text = source_code[start_byte:end_byte]
            start_line = source_code[:start_byte].count("\n") + 1
            lines = func_def_text.splitlines()
            numbered_func_def = [f"{start_line + i:>5} | {line}" for i, line in enumerate(lines)]
            return (f"File: {file_path.removeprefix(checked_src_root)}"
                    f"\n{"\n".join(numbered_func_def)}")

    def get_func_def(self, func_name, src_file):
        src_file = str(Path(self.path_mgr.get_checked_src_path()) / src_file)
        func_def_list = self.func_def_mgr.select_func(func_name)

        if not func_def_list:
            return None

        func_def = next((r for r in func_def_list if r.src_file == src_file), None)
        if func_def:
            return self.get_func_def_text(func_def)

        include_list = self.include_info_mgr.select_file(src_file) 

        if include_list:
            func_def = next((r for r in func_def_list if any(
                r.src_file in item.include_files for item in include_list)), None)
            if func_def:
                return self.get_func_def_text(func_def)

        func_def = next((r for r in func_def_list if r.is_static == 0), None)
        if func_def:
            return self.get_func_def_text(func_def)

        return None
